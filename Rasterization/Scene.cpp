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

Matrix4 calcModelingTransformations(Scene & scene) {
    // TODO: Implement dis
    Matrix4 M_model = Matrix4();
    return M_model;
}

Matrix4 calcCameraTransformation(Camera * camera) {
    // TODO: Implement dis
    Matrix4 M_cam = Matrix4();
    return M_cam;
}

Matrix4 calcProjectionTransformation(Scene & scene) {
    // TODO: Implement dis
    //    if (scene.projectionType)
    Matrix4 M_proj = Matrix4();
    return M_proj;
}

Matrix4 calcViewportTransformation(Scene & scene) {
    // TODO: Implement dis
    Matrix4 M_viewport = Matrix4();
    return M_viewport;
}

/*
	Transformations, clipping, culling, rasterization are done here.
	You can define helper functions inside Scene class implementation.
*/
// TODO: Provide a Makefile!~
// TODO: Implement this function.
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
void Scene::forwardRenderingPipeline(Camera *camera) {
    Matrix4 M_model = calcModelingTransformations(*this);
    Matrix4 M_cam = calcCameraTransformation(camera);
    Matrix4 M_proj = calcProjectionTransformation(*this);
    Matrix4 M_viewport = calcViewportTransformation(*this);

    /* For each model apply these transformations + clip + cull then rasterize */
    Matrix4 M_cam_model = multiplyMatrixWithMatrix(M_cam, M_model); // m1*m2
    Matrix4 M_proj_cam_model = multiplyMatrixWithMatrix(M_proj, M_cam_model);
    for (int i = 0; i < this->models.size(); ++i) {
        for (int j = 0; j < this->models[i]->triangles.size(); ++j) {
            Vec3 * v1 = this->vertices[this->models[i]->triangles[j].getFirstVertexId()];
            Vec3 * v2 = this->vertices[this->models[i]->triangles[j].getSecondVertexId()];
            Vec3 * v3 = this->vertices[this->models[i]->triangles[j].getThirdVertexId()];
            Vec4 projectedV1 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v1->x, v1->y, v1->z, 1, v1->colorId));
            Vec4 projectedV2 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v2->x, v2->y, v2->z, 1, v2->colorId));
            Vec4 projectedV3 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v3->x, v3->y, v3->z, 1, v3->colorId));

            if (!this->models[i]->type) {
                /* Wireframe mode */

                // Construct Lines

                // TODO: Backface CULL if enabled
                // TODO: CLIP
                // TODO: Pers Div => CHECK IF WE NEED THIS
                // TODO: Mult with M_viewport
                // TODO: Line Rasterization - Rasterize (vector<Color*> colorsofvertices) and fill this.image
            }
            else {
                /* Solid mode */

                // TODO: Backface CULL if enabled
                // TODO: Pers Div => CHECK IF WE NEED THIS
                // TODO: Mult with M_viewport
                // TODO: Triangle Rasterization - Rasterize (vector<Color*> colorsofvertices) and fill this.image
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