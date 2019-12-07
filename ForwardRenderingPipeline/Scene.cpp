#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <cmath>

#include "Scene.h"
#include "Camera.h"
#include "Color.h"
#include "Model.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "tinyxml2.h"
#include "Helpers.h"

using namespace tinyxml2;
using namespace std;

Matrix4 calcModelingTransformations(Camera * camera, const vector<Translation*>& translations,
        const vector<Rotation*>& rotations, const vector<Scaling*>& scalings) {
    // TODO: Implement dis -- GOKHAN
    // TODO: Do not forget uvw NORMALIZATIONS in operations ETC.!
    Matrix4 M_model = Matrix4();
    return M_model;
}

Matrix4 calcCameraTransformation(Camera * camera) {
    double T[4][4] = {{1, 0, 0, -(camera->pos.x)},
                        {0, 1, 0, -(camera->pos.y)},
                        {0, 0, 1, -(camera->pos.z)},
                        {0, 0, 0, 1}};
    double R[4][4] = {{camera->u.x, camera->u.y, camera->u.z, 0},
                      {camera->v.x, camera->v.y, camera->v.z, 0},
                      {camera->w.x, camera->w.y, camera->w.z, 0},
                      {0, 0, 0, 1}};
    return multiplyMatrixWithMatrix(Matrix4(T), Matrix4(R));
}

Matrix4 calcProjectionTransformation(Camera * camera, bool projType) {
    if (projType) {
        /* Return M_pers */
        double M_pers[4][4] = {{(2*camera->near)/(camera->right - camera->left), 0, (camera->right + camera->left) / (camera->right - camera->left), 0},
                               {0, (2*camera->near)/(camera->top - camera->bottom), (camera->top + camera->bottom) / (camera->top - camera->bottom), 0},
                               {0, 0, -((camera->far + camera->near) / (camera->far - camera->near)), -((2*camera->far*camera->near) / (camera->far - camera->near))},
                               {0, 0, -1, 0}};
        return Matrix4(M_pers);
    }
    else {
        /* Return M_orth */
        double M_orth[4][4] = {{2/(camera->right - camera->left), 0, 0, -((camera->right + camera->left) / (camera->right - camera->left))},
                               {0, 2/(camera->top - camera->bottom), 0, -((camera->top + camera->bottom) / (camera->top - camera->bottom))},
                               {0, 0, -(2/(camera->far - camera->near)), -((camera->far + camera->near) / (camera->far - camera->near))},
                               {0, 0, 0, 1}};
        return Matrix4(M_orth);
    }
}

Matrix4 calcViewportTransformation(Camera * camera) {
    double M_viewport[4][4] = {{camera->horRes/2.0, 0, 0, (camera->horRes-1)/2.0},
                               {0, camera->verRes/2.0, 0, (camera->verRes-1)/2.0},
                               {0, 0, 0.5, 0.5},
                               {0, 0, 0, 1}}; // TODO: We do not need last line semantically but need in code!
    return Matrix4(M_viewport);
}

bool isBackfaceCulled(Camera * camera, Vec4 & v0, Vec4 & v1, Vec4 & v2) {
    // TODO: Implement dis
    // TODO: DO VERTICES SENT HERE NEED TO BE PERSPECTIVE DIVIDED?
    return false;
}

bool visible(double den, double num, double & t_E, double & t_L) {
    /* Helper function for checking visibility of the lines - updates t_E and t_L as needed */
    double t = num / den;
    if (den > 0) {
        /* Potentially Entering */
        if (t > t_L)
            return false;
        else if (t > t_E)
            t_E = t;
    }
    else if (den < 0) {
        /* Potentially Leaving */
        if (t < t_E)
            return false;
        else if (t < t_L)
            t_L = t;
    }
    else if (num > 0) {
        /* Line is parallel to the edge and outside of the bounds */
        return false;
    }
    return true;
}

void clipLine(Camera * camera, Vec4 & v0, Vec4 & v1) {
    /* Clips given line with Liang-Barsky Algorithm in 3D
     * as a result v0 and v1 gets updated as needed */
    double t_E = 0, t_L = 1;
    double dx = v1.x - v0.x, dy = v1.y - v0.y, dz = v1.z - v0.z;
    double x_min, x_max, y_min, y_max, z_min, z_max; // TODO: fill these values! -- YAVUZ
//    bool isVisible = false;
    if (visible(dx, x_min-v0.x, t_E, t_L) && visible(-dx, v0.x-x_max, t_E, t_L)
        && visible(dy, y_min-v0.y, t_E, t_L) && visible(-dy, v0.y - y_max, t_E, t_L)
        && visible(dz, z_min-v0.z, t_E, t_L) && visible(-dz, v0.z-z_max, t_E, t_L)) {
//        isVisible = true;
        if (t_L < 1) {
            v1.x = v0.x + (dx * t_L);
            v1.y = v0.y + (dy * t_L);
            v1.z = v0.z + (dz * t_L);
        }
        if (t_E > 0) {
            v0.x = v0.x + (dx * t_E);
            v0.y = v0.y + (dy * t_E);
            v0.z = v0.z + (dz * t_E);
        }
    }
}

void rasterizeLine(vector<vector<Color>> & image, Color * c0, Color * c1, Vec4 & v0, Vec4 & v1) {
    // TODO: Implement dis using Midpoint Algorithm and fill the image's related pixels - YAVUZ
    // TODO: Be careful with the slopes of the lines! if not 0<m<1 then need to use a modified midpoint algorithm!
}

void rasterizeTriangle(vector<vector<Color>> & image, Color * c0, Color * c1, Color * c2, Vec4 & v0, Vec4 & v1, Vec4 & v2) {
    // TODO: Implement dis using Barrycentric Coordinates and fill the image's related pixels
}


/*
	Transformations, clipping, culling, rasterization are done here.
	You can define helper functions inside Scene class implementation.
*/
// TODO: DO NOT FORGET TO Provide a Makefile!~ YAVUZ
/**
 * Our Overall Rendering Pipeline - ysyesilyurt
    1- Implement Modeling Transformation Matrix Calculation => Mmodel
        - Transformation
        - Rotation
        - Scaling
    2- Implement Camera Transformation Matrix Calculation => Mcam
    3- Implement Projection Transformation Matrix Calculation => Mproj
        - Orthogonal Projection (if proj type == 0) => Morth
        - Perspective Projection (if proj type == 1) => Mpers = Morth * Mpers2orth
    4- Implement Clipping Algorithm (Sutherland's or Liang's - possibly Liang's)
    5- Implement Backface Culling
    6- Perspective Divide (if perspective projection has been implemented) => /w
    7- Implement Viewport Transformation Matrix Calculation => Mvp
    8- Implement Rasterization
        - Line Rasterization => Midpoint Algorithm => BE CAREFUL WITH SLOPES !!!!!!!!
        - Triangle Rasterization => Barrycentric Coordinates

    => Finally integrate these implementations in forwardRenderingPipeline() as: Vertice->1->2->3->4->5->6->7->8
    => Namely: Vertice * Mmodel * Mcam * Mproj -> Clipping & Culling & Pers Divide -> * Mviewport -> Rasterization

    Notes:
    -> Models have 2 types:
        1- Solid mode (if type == 1)
        2- Wireframe mode (if type == 0)
    -> Clipping will be applied for only Wireframe mode
    -> Only Backface culling will be implemented and it can be disabled/enabled (Default is disabled!)
    -> Helpers such as normalization, dot product etc. are given to us in Helpers.h/cpp
 */
void Scene::forwardRenderingPipeline(Camera * camera) {
    Matrix4 M_model = calcModelingTransformations(camera, this->translations, this->rotations, this->scalings);
    Matrix4 M_cam = calcCameraTransformation(camera);
    Matrix4 M_proj = calcProjectionTransformation(camera, this->projectionType);
    Matrix4 M_viewport = calcViewportTransformation(camera); // Normally this matrix is 3x4 but in code it is 4x4 we keep this matrix's last row as 0 0 0 1

    /* For each model apply these transformations + clip + cull then rasterize */
    Matrix4 M_cam_model = multiplyMatrixWithMatrix(M_cam, M_model); // m1*m2
    Matrix4 M_proj_cam_model = multiplyMatrixWithMatrix(M_proj, M_cam_model);
    for (int i = 0; i < this->models.size(); ++i) {
        for (int j = 0; j < this->models[i]->triangles.size(); ++j) {
            Vec3 * v0 = this->vertices[this->models[i]->triangles[j].getFirstVertexId()];
            Vec3 * v1 = this->vertices[this->models[i]->triangles[j].getSecondVertexId()];
            Vec3 * v2 = this->vertices[this->models[i]->triangles[j].getThirdVertexId()];
            Color * c0 = this->colorsOfVertices[this->models[i]->triangles[j].getFirstVertexId()];
            Color * c1 = this->colorsOfVertices[this->models[i]->triangles[j].getSecondVertexId()];
            Color * c2 = this->colorsOfVertices[this->models[i]->triangles[j].getThirdVertexId()];

            Vec4 projectedV0 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v0->x, v0->y, v0->z, 1, v0->colorId));
            Vec4 projectedV1 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v1->x, v1->y, v1->z, 1, v1->colorId));
            Vec4 projectedV2 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v2->x, v2->y, v2->z, 1, v2->colorId));

            /* Culling Phase if enabled */
            if (this->cullingEnabled && isBackfaceCulled(camera, projectedV0, projectedV1, projectedV2)) {
                /* If backface culling is enabled and model is backfacing then just continue, do not render this model */
                continue;
            }

            if (!this->models[i]->type) {
                /* Wireframe mode */

                /* Clipping Phase */
                // Create copies of vertices since their coords may change as clipping continues
                Vec4 _projectedV0 = projectedV0;
                Vec4 _projectedV1 = projectedV1;
                Vec4 _projectedV2 = projectedV2;

                /* Construct Lines to be clipped */
                // Line-1 => L01
                clipLine(camera, projectedV0, projectedV1);

                // Line-2 => L12
                clipLine(camera, _projectedV1, projectedV2);

                // Line-3 => L20
                clipLine(camera, _projectedV2, _projectedV0);

                // TODO: Pers Div => CHECK IF WE NEED THIS ????

                /* Viewport Transformation Phase */
                // L01
                Vec4 viewportV0 = multiplyMatrixWithVec4(M_viewport, projectedV0);
                Vec4 viewportV1 = multiplyMatrixWithVec4(M_viewport, projectedV1);

                // L12
                Vec4 _viewportV1 = multiplyMatrixWithVec4(M_viewport, _projectedV1);
                Vec4 viewportV2 = multiplyMatrixWithVec4(M_viewport, projectedV2);

                // L20
                Vec4 _viewportV2 = multiplyMatrixWithVec4(M_viewport, _projectedV2);
                Vec4 _viewportV0 = multiplyMatrixWithVec4(M_viewport, _projectedV0);

                /* Final step of FRP - Rasterize the Line and fill the image for this model */
                rasterizeLine(this->image, c0, c1, viewportV0, viewportV1); // L01
                rasterizeLine(this->image, c1, c2, _viewportV1, viewportV2); // L12
                rasterizeLine(this->image, c2, c0, _viewportV2, _viewportV0); // L20
            }
            else {
                /* Solid mode */

                // TODO: Pers Div => CHECK IF WE NEED THIS ????

                /* Viewport Transformation Phase */
                Vec4 viewportV0 = multiplyMatrixWithVec4(M_viewport, projectedV0);
                Vec4 viewportV1 = multiplyMatrixWithVec4(M_viewport, projectedV1);
                Vec4 viewportV2 = multiplyMatrixWithVec4(M_viewport, projectedV2);

                /* Final step of FRP - Rasterize the Triangle and fill the image for this model */
                rasterizeTriangle(this->image, c0, c1, c2, viewportV0, viewportV1, viewportV2);
            }
        }
    }
}

/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *pElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *pRoot = xmlDoc.FirstChild();

	// read background color
	pElement = pRoot->FirstChildElement("BackgroundColor");
	str = pElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	pElement = pRoot->FirstChildElement("Culling");
	if (pElement != NULL)
		pElement->QueryBoolText(&cullingEnabled);

	// read projection type
	pElement = pRoot->FirstChildElement("ProjectionType");
	if (pElement != NULL)
		pElement->QueryIntText(&projectionType);

	// read cameras
	pElement = pRoot->FirstChildElement("Cameras");
	XMLElement *pCamera = pElement->FirstChildElement("Camera");
	XMLElement *camElement;
	while (pCamera != NULL)
	{
		Camera *cam = new Camera();

		pCamera->QueryIntAttribute("id", &cam->cameraId);

		camElement = pCamera->FirstChildElement("Position");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->pos.x, &cam->pos.y, &cam->pos.z);

		camElement = pCamera->FirstChildElement("Gaze");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->gaze.x, &cam->gaze.y, &cam->gaze.z);

		camElement = pCamera->FirstChildElement("Up");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->v.x, &cam->v.y, &cam->v.z);

		cam->gaze = normalizeVec3(cam->gaze);
		cam->u = crossProductVec3(cam->gaze, cam->v);
		cam->u = normalizeVec3(cam->u);

		cam->w = inverseVec3(cam->gaze);
		cam->v = crossProductVec3(cam->u, cam->gaze);
		cam->v = normalizeVec3(cam->v);

		camElement = pCamera->FirstChildElement("ImagePlane");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &cam->left, &cam->right, &cam->bottom, &cam->top,
			   &cam->near, &cam->far, &cam->horRes, &cam->verRes);

		camElement = pCamera->FirstChildElement("OutputName");
		str = camElement->GetText();
		cam->outputFileName = string(str);

		cameras.push_back(cam);

		pCamera = pCamera->NextSiblingElement("Camera");
	}

	// read vertices
	pElement = pRoot->FirstChildElement("Vertices");
	XMLElement *pVertex = pElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (pVertex != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = pVertex->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = pVertex->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		vertices.push_back(vertex);
		colorsOfVertices.push_back(color);

		pVertex = pVertex->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	pElement = pRoot->FirstChildElement("Translations");
	XMLElement *pTranslation = pElement->FirstChildElement("Translation");
	while (pTranslation != NULL)
	{
		Translation *translation = new Translation();

		pTranslation->QueryIntAttribute("id", &translation->translationId);

		str = pTranslation->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		translations.push_back(translation);

		pTranslation = pTranslation->NextSiblingElement("Translation");
	}

	// read scalings
	pElement = pRoot->FirstChildElement("Scalings");
	XMLElement *pScaling = pElement->FirstChildElement("Scaling");
	while (pScaling != NULL)
	{
		Scaling *scaling = new Scaling();

		pScaling->QueryIntAttribute("id", &scaling->scalingId);
		str = pScaling->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		scalings.push_back(scaling);

		pScaling = pScaling->NextSiblingElement("Scaling");
	}

	// read rotations
	pElement = pRoot->FirstChildElement("Rotations");
	XMLElement *pRotation = pElement->FirstChildElement("Rotation");
	while (pRotation != NULL)
	{
		Rotation *rotation = new Rotation();

		pRotation->QueryIntAttribute("id", &rotation->rotationId);
		str = pRotation->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		rotations.push_back(rotation);

		pRotation = pRotation->NextSiblingElement("Rotation");
	}

	// read models
	pElement = pRoot->FirstChildElement("Models");

	XMLElement *pModel = pElement->FirstChildElement("Model");
	XMLElement *modelElement;
	while (pModel != NULL)
	{
		Model *model = new Model();

		pModel->QueryIntAttribute("id", &model->modelId);
		pModel->QueryIntAttribute("type", &model->type);

		// read model transformations
		XMLElement *pTransformations = pModel->FirstChildElement("Transformations");
		XMLElement *pTransformation = pTransformations->FirstChildElement("Transformation");

		pTransformations->QueryIntAttribute("count", &model->numberOfTransformations);

		while (pTransformation != NULL)
		{
			char transformationType;
			int transformationId;

			str = pTransformation->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			model->transformationTypes.push_back(transformationType);
			model->transformationIds.push_back(transformationId);

			pTransformation = pTransformation->NextSiblingElement("Transformation");
		}

		// read model triangles
		XMLElement *pTriangles = pModel->FirstChildElement("Triangles");
		XMLElement *pTriangle = pTriangles->FirstChildElement("Triangle");

		pTriangles->QueryIntAttribute("count", &model->numberOfTriangles);

		while (pTriangle != NULL)
		{
			int v1, v2, v3;

			str = pTriangle->GetText();
			sscanf(str, "%d %d %d", &v1, &v2, &v3);

			model->triangles.push_back(Triangle(v1, v2, v3));

			pTriangle = pTriangle->NextSiblingElement("Triangle");
		}

		models.push_back(model);

		pModel = pModel->NextSiblingElement("Model");
	}
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
			}

			this->image.push_back(rowOfColors);
		}
	}
	// if image is filled before, just change color rgb values with the background color
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				this->image[i][j].r = this->backgroundColor.r;
				this->image[i][j].g = this->backgroundColor.g;
				this->image[i][j].b = this->backgroundColor.b;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFileName.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFileName << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
	os_type == 1 		-> Ubuntu
	os_type == 2 		-> Windows
	os_type == other	-> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType)
{
	string command;

	// call command on Ubuntu
	if (osType == 1)
	{
		command = "convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// call command on Windows
	else if (osType == 2)
	{
		command = "magick convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// default action - don't do conversion
	else
	{
	}
}