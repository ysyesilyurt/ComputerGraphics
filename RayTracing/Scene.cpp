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
const float INF = numeric_limits<float>::max();

IntersectionData intersectRay(const Ray & ray, const vector<Shape *> & objects) {

    /* Calculate the nearest intersection point calling Shape's intersect with given ray */

    IntersectionData minIntersection = {INF, {}, -1};
    // For each object in the scene Intersect ray with all the shapes in scene and get the nearest one
    for (int i = 0; i < objects.size(); ++i) {
        IntersectionData tempIntersection = objects[i]->intersect(ray); // calling object's own intersect method
        if (tempIntersection.t != INF) {
            if (minIntersection.t > tempIntersection.t) {
                minIntersection = tempIntersection;
            }
        }
    }
    return minIntersection;
}

Vector3f computeSpecular(const Material * material, const Vector3f & normalVector,
        const Vector3f & irradiance, const Vector3f & halfVector) {

    // (cosAlpha)^ns
    float phongExponentCosAlpha = pow(max(0.0f, dotProduct(normalVector, halfVector)), material->phongExp);
    // (cosAlpha)^ns * E(d)
    Vector3f specularWithoutCoeffs = irradiance * phongExponentCosAlpha;
    // Multiplying with specular coeff
    specularWithoutCoeffs.r *= material->specularRef.r;
    specularWithoutCoeffs.g *= material->specularRef.g;
    specularWithoutCoeffs.b *= material->specularRef.b;

    return specularWithoutCoeffs;
}

Vector3f computeDiffuse(const Material * material, const Vector3f & normalVector,
        const Vector3f & lightDirection, const Vector3f & irradiance) {

    // cosTheta
    float cosTheta = max(0.0f, dotProduct(normalize(lightDirection), normalVector));
    // cosTheta * E(d)
    Vector3f diffuseWithoutCoeffs = irradiance * cosTheta;
    // Multiplying with diffuse coeff
    diffuseWithoutCoeffs.r *= material->diffuseRef.r;
    diffuseWithoutCoeffs.g *= material->diffuseRef.g;
    diffuseWithoutCoeffs.b *= material->diffuseRef.b;

    return diffuseWithoutCoeffs;
}

Vector3f computeAmbient(const Material * material, const Vector3f & ambientLight) {
    return {material->ambientRef.r * ambientLight.r,
            material->ambientRef.g * ambientLight.g,
            material->ambientRef.b * ambientLight.b};
}

Vector3f computeRadiance(const Ray & ray, const IntersectionData & intersection, Scene * scene, int remainingRecursion) {

    Vector3f pixelColor = {};
    Material * intersectionMaterial = scene->materials[intersection.materialId - 1];

    // Calculate Ambient shading and add it to pixelColor (adding ambient directly to all pixels)
    Vector3f ambientContribution = computeAmbient(intersectionMaterial, scene->ambientLight);
    pixelColor = ambientContribution;

    Vector3f intersectionPoint = ray.origin + (ray.direction * intersection.t);
    // subtract intPoint from camera's position (origin) and find the vector that goes to eye
    Vector3f eyeVector = ray.origin - intersectionPoint; // w_0

    // For each light i
    for (int i = 0; i < scene->lights.size(); ++i) {

        Vector3f lightDirection = scene->lights[i]->position - intersectionPoint;
        Vector3f normalizedLightDirection = normalize(lightDirection);

        // Cast the shadow ray s from intersection point to i
        Ray shadowRay;
        Vector3f intOffset = normalizedLightDirection * scene->shadowRayEps; // moving intPoint a bit further to avoid fp precision errors
        shadowRay.origin = intersectionPoint + intOffset;
        shadowRay.direction = normalizedLightDirection;

        // Intersect s with all objects again to check if there is any obj between the light source and point
        IntersectionData shadowIntersection = intersectRay(shadowRay, scene->objects);
        if (shadowIntersection.t >= vectorLength(lightDirection)) {
            // If there is not an intersection between the light source and point
            // Then there is contribution from this light source -- point is not in shadow

            /* Calculate diffuse and specular shading from this light source and add them to the pixelColor */

            // Compute irradiance of light source i on intersection point
            Vector3f irradiance = scene->lights[i]->computeLightContribution(intersectionPoint);

            // Compute Diffuse
            Vector3f diffuseContribution = computeDiffuse(intersectionMaterial, intersection.normal,
                                                       lightDirection, irradiance);
            pixelColor += diffuseContribution;

            // Compute Specular
            Vector3f normalizedHalfVector = normalize(normalizedLightDirection + normalize(eyeVector));
            Vector3f specularContribution = computeSpecular(intersectionMaterial, intersection.normal,
                    irradiance, normalizedHalfVector);
            pixelColor += specularContribution;
        }
        else {
            // Else there is an intersection continue -- point is in shadow -- no contribution from this light
            continue;
        }
    }

    // Check if the material of intersected object has a nonzero MirrorReflectance value
    // Then Bounce primary ray until no intersection or maxRecDepth (count is initially zero)

    if ((intersectionMaterial->mirrorRef.x > 0 || intersectionMaterial->mirrorRef.y > 0 ||
    intersectionMaterial->mirrorRef.z > 0) && remainingRecursion > 0) {

        // Calculate reflected ray's direction using w_r = -w_0 + 2*n*cosTheta => cosTheta = n.w_0
        // Also move set its origin as intersectionPoint which is moved a bit further by shadowRayEps
        Ray reflectedRay;
        float cosTheta = dotProduct(intersection.normal, ray.direction);
        reflectedRay.direction = ((ray.direction * -1) + (intersection.normal * (2 * cosTheta))) * -1; // todo: why do we multiply with -1 at the end?
        reflectedRay.origin = intersectionPoint + (reflectedRay.direction * scene->shadowRayEps);

        // Again Calculate the nearest intersection of reflected Ray
        IntersectionData reflectedIntersection = intersectRay(reflectedRay, scene->objects);

        if (reflectedIntersection.t != INF) { // means that ray hit an object
            Vector3f reflectedRadiance = computeRadiance(reflectedRay, reflectedIntersection, scene,
                    remainingRecursion-1);

            reflectedRadiance.x *= intersectionMaterial->mirrorRef.x;
            reflectedRadiance.y *= intersectionMaterial->mirrorRef.y;
            reflectedRadiance.z *= intersectionMaterial->mirrorRef.z;
            pixelColor += reflectedRadiance;
        }
    }

    // Dont forget to clamp the resulting pixelColor
    pixelColor.r = min(max(0.0f, pixelColor.r), 255.0f);
    pixelColor.g = min(max(0.0f, pixelColor.g), 255.0f);
    pixelColor.b = min(max(0.0f, pixelColor.b), 255.0f);
    return pixelColor;
}

Color renderPixel(int row, int col, Scene * scene, int camIndex) {

    // Calculate primary ray from Camera x that goes through pixel
    Ray primRay = scene->cameras[camIndex]->getPrimaryRay(row, col);

    // Calculate nearest intersection
    IntersectionData intersection = intersectRay(primRay, scene->objects);

    if (intersection.t != INF) { // means that ray hit an object
        Vector3f pxColor = computeRadiance(primRay, intersection, scene, scene->maxRecursionDepth);
        return {static_cast<unsigned char>(pxColor.r),
                static_cast<unsigned char>(pxColor.g),
                static_cast<unsigned char>(pxColor.b)};
    }
    else { // no intersection, just set the pixel's color to background color
        return {static_cast<unsigned char>(scene->backgroundColor.r),
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
            image->setPixelValue(i, j, colorOfPixel);
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
     Ray trace for each camera x:
        Create an Image instance (according to ImagePlane values of camera x)
            For each pixel i in Image:
                Calculate primary ray from Camera x that goes through pixel i
                Calculate the nearest intersection point calling Shape's intersect with calculated primary ray
                    - Cast shadow rays to each light source from calculated intersection point
                    - Calculate Basic Illumination Model of Material Shading properties Diffuse, Ambient, Specular
                    - Recursively track the ray acc. to maxRecDepth
                Compute rgb value of pixel i according to results and fill it in Image instance
         Call save image and save the image
     */

    for (int x = 0; x < cameras.size(); ++x) {
<<<<<<< HEAD
        Image * image = new Image(abs(cameras[x]->imgPlane.left - cameras[x]->imgPlane.right),
                                  abs(cameras[x]->imgPlane.top - cameras[x]->imgPlane.bottom));

        image->data = new Color*[cameras[x]->imgPlane.nx];
        for(int y = 0; y < cameras[x]->imgPlane.nx; ++y)
            image->data[y] = new Color[cameras[x]->imgPlane.ny];

=======
        Image * image = new Image(cameras[x]->imgPlane.nx,cameras[x]->imgPlane.ny);
>>>>>>> 7562923074aa679c8e20bbc1044202d037f5ab4c
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

