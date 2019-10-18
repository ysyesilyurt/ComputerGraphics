#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Shape.h"
#include "tinyxml2.h"
#include "Image.h"
#include "helpers.h"
#include <limits>

using namespace tinyxml2;
const float INFINITY = numeric_limits<float>::max();

IntersectionData intersectRay(const Ray & ray, const vector<Shape *> & objects) {

    // Calculate the nearest intersection point calling Shape's intersect with calculated primary ray
    IntersectionData intersection = {INFINITY, Vector3f{}, -1};
    // For each object in the scene
    // Intersect all shapes in scene with primRay and gather all IntersectionDatas
    for (int k = 0; k < objects.size(); ++k) {
        // TODO: Implement this function
        // if ray intersects k
        // if t < intersection.t
        // intersection.t = t
        // update normal and materialId
    }
    return intersection;
}

Color computeSpecular(const Material * material, const Vector3f & normalVector,
                     const Vector3f & lightDirection, const Vector3f & irradiance, const Vector3f & halfVector) {
    // (cosAlpha)^ns
    float phongExponentCosAlpha = std::pow(std::max(0.0f, dotProduct(lightDirection, normalVector)), material->phongExp);
    // (cosAlpha)^ns * E(d)
    Vector3f specularWithoutCoeffs = irradiance * phongExponentCosAlpha;
    // Multiplying with specular coeff
    specularWithoutCoeffs.r *= material->specularRef.r
    specularWithoutCoeffs.g *= material->specularRef.g
    specularWithoutCoeffs.b *= material->specularRef.b

    return specularWithoutCoeffs; // TODO casting V3f to Color!
}

Color computeDiffuse(const Material * material, const Vector3f & normalVector,
        const Vector3f & lightDirection, const Vector3f & irradiance) {
    // cosTheta
    float cosTheta = std::max(0.0f, dotProduct(lightDirection, normalVector));
    // cosTheta * E(d)
    Vector3f diffuseWithoutCoeffs = irradiance * cosTheta;
    // Multiplying with diffuse coeff
    diffuseWithoutCoeffs.r *= material->diffuseRef.r
    diffuseWithoutCoeffs.g *= material->diffuseRef.g
    diffuseWithoutCoeffs.b *= material->diffuseRef.b

    return diffuseWithoutCoeffs; // TODO casting V3f to Color!
}

Color computeAmbient(const Material * material, const Vector3f & ambientLight) {
    return {material->ambientRef.r * ambientLight.r,
            material->ambientRef.g * ambientLight.g,
            material->ambientRef.b * ambientLight.b}; // check if we need xyz or not?
}

Color calculateRadiance(const Ray & ray, const IntersectionData & intersection, Scene * scene) {

    Color pixelColor = {};
    Material * material = scene->materials[intersection.materialId - 1];

    // Calculate Ambient shading and add it to pixelColor (adding ambient directly to all pixels)
    Color ambientContribution = computeAmbient(material, ambientLight);
    pixelColor = ambientContribution;

    Vector3f intersectionPoint = ray.origin + ray.direction * intersection.t;
    Vector3f normalizedEyeVector = (ray.origin - intersectionPoint).normalize(); // subtract intPoint from camera's position (origin) and find w_0

    // For each light i
    for (int i = 0; i < scene->lights.size(); ++i) {
        Vector3f normalizedLightDirection = (scene->lights[i]->position - intersectionPoint).normalize();

        // Cast the shadow ray s from intersection point to i
        Ray shadowRay;
        Vector3f intOffset = scene->shadowRayEps * normalizedLightDirection;
        shadowRay.origin = intersectionPoint + intOffset;
        shadowRay.direction = normalizedLightDirection;

        // Intersect s with all objects again to check if there is any obj between the light source and point
        IntersectionData shadowIntersection = intersectRay(shadowRay, scene->objects);

        // If there is an intersection continue -- point is in shadow -- no contribution from this light
        if (shadowIntersection.t == INFINITY) // TODO: GUY CHECKED HERE DIFFERENTLY? CHECK HERE!
            continue;
        else {
            // Else calculate diffuse and specular shading from this light source and add them to the pixelColor
            // -- there is contribution from this light source -- point is not in shadow

            // Compute irradiance of light source i on intersection point
            Vector3f irradianceContribution += scene->lights[i]->computeLightContribution(intersectionPoint);

            // computeDiffuse
            Vector3f diffuseContribution = computeDiffuse(material, intersection.normal,
                                                          normalizedLightDirection, irradianceContribution);
            pixelColor += diffuseContribution;

            // computeSpecular
            Vector3f normalizedHalfVector = (normalizedLightDirection + normalizedEyeVector).normalize();
            Vector3f specularContribution = computeSpecular(material, intersection.normal, normalizedLightDirection,
                    irradianceContribution, normalizedHalfVector);
            pixelColor += specularContribution;
        }
    }

    // TODO: Handle reflectance of pixel

    // TODO: dont forget to clamp the resulting pixelColor
    return pixelColor;
}

Color renderPixel(int row, int col, Scene * scene, int camIndex) {

    // Calculate primary ray from Camera x that goes through pixel
    Ray primRay = scene->cameras[camIndex]->getPrimaryRay(row, col);

    // Calculate nearest intersection
    IntersectionData intersection = intersectRay(primRay, scene->objects);

    if (intersection.t != INFINITY) { // means that ray hit an object
        return calculateRadiance(primRay, intersection, scene);
    }
    else { // no intersection, just set the pixel's color to background color
        return { static_cast<unsigned char>(scene->backgroundColor.r),
                 static_cast<unsigned char>(scene->backgroundColor.g),
                 static_cast<unsigned char>(scene->backgroundColor.b)};
    }
}

void rayTracing(Image * image, Scene * scene, int camIndex) {

    // TODO: After basic functionality is implemented -- implement below section such that all threads get 1 row and handle it

    // For each pixel in Image
    for (int i = 0; i < scene->cameras[camIndex]->imgPlane.nx; ++i) {
        for (int j = 0; j < scene->cameras[camIndex]->imgPlane.ny; ++j) {
            Color colorOfPixel = renderPixel(i, j, scene, camIndex);
            image->setPixelValue(j, i, colorOfPixel);
        }
    }
}

/*
 * Must render the scene from each camera's viewpoint and create an image.
 * You can use the methods of the Image class to save the image as a PPM file.
 */
void Scene::renderScene(void)
{
    /**
     * TODO
     Ray trace for each camera x:
        Create an Image instance (according to ImagePlane values of camera x)
            For each pixel i in Image:
                Calculate primary ray from Camera x that goes through pixel i
                Calculate the nearest intersection point calling Shape's intersect with calculated primary ray
                    - Recursively track the ray acc. to maxRecDepth
                    - Generate shadow rays to each light source from calculated intersection point (using Light's computeLightCont ?)
                        ? - Basic Illumination Model of Material properties Diffuse, Ambient, Specular ?
                Compute rgb value of pixel i according to results and fill it in Image instance
         Call save image and save the image
     */

    for (int x = 0; x < cameras.size(); ++x) {
        Image * image = new Image(cameras[x]->imgPlane.left - cameras[x]->imgPlane.right,
                                  cameras[x]->imgPlane.top - cameras[x]->imgPlane.bottom);

        image->data = new Color*[cameras[x]->imgPlane.nx];
        for(int y = 0; y < cameras[x]->imgPlane.nx; ++y)
            image->data[y] = new Color[cameras[x]->imgPlane.ny];

        rayTracing(image, this, x);
        image->saveImage(cameras[x]->imageName);
    }
}

// Parses XML file. 
Scene::Scene(const char *xmlPath)
{
    const char *str;
    XMLDocument xmlDoc;
    XMLError eResult;
    XMLElement *pElement;

    maxRecursionDepth = 1;
    shadowRayEps = 0.001;

    eResult = xmlDoc.LoadFile(xmlPath);

    XMLNode *pRoot = xmlDoc.FirstChild();

    pElement = pRoot->FirstChildElement("MaxRecursionDepth");
    if(pElement != nullptr)
        pElement->QueryIntText(&maxRecursionDepth);

    pElement = pRoot->FirstChildElement("BackgroundColor");
    str = pElement->GetText();
    sscanf(str, "%f %f %f", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

    pElement = pRoot->FirstChildElement("ShadowRayEpsilon");
    if(pElement != nullptr)
        pElement->QueryFloatText(&shadowRayEps);

    pElement = pRoot->FirstChildElement("IntersectionTestEpsilon");
    if(pElement != nullptr)
        eResult = pElement->QueryFloatText(&intTestEps);

    // Parse cameras
    pElement = pRoot->FirstChildElement("Cameras");
    XMLElement *pCamera = pElement->FirstChildElement("Camera");
    XMLElement *camElement;
    while(pCamera != nullptr)
    {
        int id;
        char imageName[64];
        Vector3f pos, gaze, up;
        ImagePlane imgPlane;

        eResult = pCamera->QueryIntAttribute("id", &id);
        camElement = pCamera->FirstChildElement("Position");
        str = camElement->GetText();
        sscanf(str, "%f %f %f", &pos.x, &pos.y, &pos.z);
        camElement = pCamera->FirstChildElement("Gaze");
        str = camElement->GetText();
        sscanf(str, "%f %f %f", &gaze.x, &gaze.y, &gaze.z);
        camElement = pCamera->FirstChildElement("Up");
        str = camElement->GetText();
        sscanf(str, "%f %f %f", &up.x, &up.y, &up.z);
        camElement = pCamera->FirstChildElement("NearPlane");
        str = camElement->GetText();
        sscanf(str, "%f %f %f %f", &imgPlane.left, &imgPlane.right, &imgPlane.bottom, &imgPlane.top);
        camElement = pCamera->FirstChildElement("NearDistance");
        eResult = camElement->QueryFloatText(&imgPlane.distance);
        camElement = pCamera->FirstChildElement("ImageResolution");
        str = camElement->GetText();
        sscanf(str, "%d %d", &imgPlane.nx, &imgPlane.ny);
        camElement = pCamera->FirstChildElement("ImageName");
        str = camElement->GetText();
        strcpy(imageName, str);

        cameras.push_back(new Camera(id, imageName, pos, gaze, up, imgPlane));

        pCamera = pCamera->NextSiblingElement("Camera");
    }

    // Parse materals
    pElement = pRoot->FirstChildElement("Materials");
    XMLElement *pMaterial = pElement->FirstChildElement("Material");
    XMLElement *materialElement;
    while(pMaterial != nullptr)
    {
        materials.push_back(new Material());

        int curr = materials.size() - 1;

        eResult = pMaterial->QueryIntAttribute("id", &materials[curr]->id);
        materialElement = pMaterial->FirstChildElement("AmbientReflectance");
        str = materialElement->GetText();
        sscanf(str, "%f %f %f", &materials[curr]->ambientRef.r, &materials[curr]->ambientRef.g, &materials[curr]->ambientRef.b);
        materialElement = pMaterial->FirstChildElement("DiffuseReflectance");
        str = materialElement->GetText();
        sscanf(str, "%f %f %f", &materials[curr]->diffuseRef.r, &materials[curr]->diffuseRef.g, &materials[curr]->diffuseRef.b);
        materialElement = pMaterial->FirstChildElement("SpecularReflectance");
        str = materialElement->GetText();
        sscanf(str, "%f %f %f", &materials[curr]->specularRef.r, &materials[curr]->specularRef.g, &materials[curr]->specularRef.b);
        materialElement = pMaterial->FirstChildElement("MirrorReflectance");
        if(materialElement != nullptr)
        {
            str = materialElement->GetText();
            sscanf(str, "%f %f %f", &materials[curr]->mirrorRef.r, &materials[curr]->mirrorRef.g, &materials[curr]->mirrorRef.b);
        }
        else
        {
            materials[curr]->mirrorRef.r = 0.0;
            materials[curr]->mirrorRef.g = 0.0;
            materials[curr]->mirrorRef.b = 0.0;
        }
        materialElement = pMaterial->FirstChildElement("PhongExponent");
        if(materialElement != nullptr)
            materialElement->QueryIntText(&materials[curr]->phongExp);

        pMaterial = pMaterial->NextSiblingElement("Material");
    }

    // Parse vertex data
    pElement = pRoot->FirstChildElement("VertexData");
    int cursor = 0;
    Vector3f tmpPoint;
    str = pElement->GetText();
    while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
        cursor++;
    while(str[cursor] != '\0')
    {
        for(int cnt = 0 ; cnt < 3 ; cnt++)
        {
            if(cnt == 0)
                tmpPoint.x = atof(str + cursor);
            else if(cnt == 1)
                tmpPoint.y = atof(str + cursor);
            else
                tmpPoint.z = atof(str + cursor);
            while(str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
                cursor++;
            while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
                cursor++;
        }
        vertices.push_back(tmpPoint);
    }

    // Parse objects
    pElement = pRoot->FirstChildElement("Objects");

    // Parse spheres
    XMLElement *pObject = pElement->FirstChildElement("Sphere");
    XMLElement *objElement;
    while(pObject != nullptr)
    {
        int id;
        int matIndex;
        int cIndex;
        float R;

        eResult = pObject->QueryIntAttribute("id", &id);
        objElement = pObject->FirstChildElement("Material");
        eResult = objElement->QueryIntText(&matIndex);
        objElement = pObject->FirstChildElement("Center");
        eResult = objElement->QueryIntText(&cIndex);
        objElement = pObject->FirstChildElement("Radius");
        eResult = objElement->QueryFloatText(&R);

        objects.push_back(new Sphere(id, matIndex, cIndex, R));

        pObject = pObject->NextSiblingElement("Sphere");
    }

    // Parse triangles
    pObject = pElement->FirstChildElement("Triangle");
    while(pObject != nullptr)
    {
        int id;
        int matIndex;
        int p1Index;
        int p2Index;
        int p3Index;

        eResult = pObject->QueryIntAttribute("id", &id);
        objElement = pObject->FirstChildElement("Material");
        eResult = objElement->QueryIntText(&matIndex);
        objElement = pObject->FirstChildElement("Indices");
        str = objElement->GetText();
        sscanf(str, "%d %d %d", &p1Index, &p2Index, &p3Index);

        objects.push_back(new Triangle(id, matIndex, p1Index, p2Index, p3Index));

        pObject = pObject->NextSiblingElement("Triangle");
    }

    // Parse meshes
    pObject = pElement->FirstChildElement("Mesh");
    while(pObject != nullptr)
    {
        int id;
        int matIndex;
        int p1Index;
        int p2Index;
        int p3Index;
        int cursor = 0;
        int vertexOffset = 0;
        vector<Triangle> faces;

        eResult = pObject->QueryIntAttribute("id", &id);
        objElement = pObject->FirstChildElement("Material");
        eResult = objElement->QueryIntText(&matIndex);
        objElement = pObject->FirstChildElement("Faces");
        objElement->QueryIntAttribute("vertexOffset", &vertexOffset);
        str = objElement->GetText();
        while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
            cursor++;
        while(str[cursor] != '\0')
        {
            for(int cnt = 0 ; cnt < 3 ; cnt++)
            {
                if(cnt == 0)
                    p1Index = atoi(str + cursor) + vertexOffset;
                else if(cnt == 1)
                    p2Index = atoi(str + cursor) + vertexOffset;
                else
                    p3Index = atoi(str + cursor) + vertexOffset;
                while(str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
                    cursor++;
                while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
                    cursor++;
            }
            faces.push_back(*(new Triangle(-1, matIndex, p1Index, p2Index, p3Index)));
        }

        objects.push_back(new Mesh(id, matIndex, faces));

        pObject = pObject->NextSiblingElement("Mesh");
    }

    // Parse lights
    int id;
    Vector3f position;
    Vector3f intensity;
    pElement = pRoot->FirstChildElement("Lights");

    XMLElement *pLight = pElement->FirstChildElement("AmbientLight");
    XMLElement *lightElement;
    str = pLight->GetText();
    sscanf(str, "%f %f %f", &ambientLight.r, &ambientLight.g, &ambientLight.b);

    pLight = pElement->FirstChildElement("PointLight");
    while(pLight != nullptr)
    {
        eResult = pLight->QueryIntAttribute("id", &id);
        lightElement = pLight->FirstChildElement("Position");
        str = lightElement->GetText();
        sscanf(str, "%f %f %f", &position.x, &position.y, &position.z);
        lightElement = pLight->FirstChildElement("Intensity");
        str = lightElement->GetText();
        sscanf(str, "%f %f %f", &intensity.r, &intensity.g, &intensity.b);

        lights.push_back(new PointLight(position, intensity));

        pLight = pLight->NextSiblingElement("PointLight");
    }
}

