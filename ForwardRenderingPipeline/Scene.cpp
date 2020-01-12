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

Matrix4 calcModelingTransMatrix(Camera * camera, Model & model, const vector<Translation*>& translations,
        const vector<Rotation*>& rotations, const vector<Scaling*>& scalings) {
    /* Create Modeling Transformation Matrix that is constructed with correct order of transformations and return it */
    Matrix4 M_model = getIdentityMatrix();
    for (int i = 0; i < model.numberOfTransformations; ++i) {
        if (model.transformationTypes[i] == 't') {
            Translation * t = translations[model.transformationIds[i]-1]; // since transformations start from 1
            double tMatrix[4][4] = {{1,0,0,t->tx},
                                   {0,1,0,t->ty},
                                   {0,0,1,t->tz},
                                   {0,0,0,1}};
            M_model = multiplyMatrixWithMatrix(tMatrix, M_model);
        }
        else if (model.transformationTypes[i] == 's') {
            Scaling * s = scalings[model.transformationIds[i]-1]; // since transformations start from 1
            double sMatrix[4][4] = {{s->sx,0,0,0},
                                    {0,s->sy,0,0},
                                    {0,0,s->sz,0},
                                    {0,0,0,1}};
            M_model = multiplyMatrixWithMatrix(sMatrix, M_model);
        }
        else if (model.transformationTypes[i] == 'r') {
            Rotation * r = rotations[model.transformationIds[i]-1]; // since transformations start from 1
            /* First find ONB uvw and construct M(mMatrix) */
            Vec3 u = Vec3(r->ux, r->uy, r->uz, -1), v, w;
            double minComp = std::min(std::min(abs(r->ux), abs(r->uy)), abs(r->uz));
            if (minComp == abs(r->ux))
                v = Vec3(0, -1 * r->uz, r->uy, -1);
            else if (minComp == abs(r->uy))
                v = Vec3(-1 * r->uz, 0, r->ux, -1);
            else if (minComp == abs(r->uz))
                v = Vec3(-1 * r->uy, r->ux, 0, -1);
            w = crossProductVec3(u, v);
            // Do not forget to normalize v and w
            v = normalizeVec3(v);
            w = normalizeVec3(w);
            double mMatrix[4][4] = {{u.x,u.y,u.z,0},
                                    {v.x,v.y,v.z,0},
                                    {w.x,w.y,w.z,0},
                                    {0,0,0,1}};
            double mMatrix_inverse[4][4] = {{u.x,v.x,w.x,0},
                                            {u.y,v.y,w.y,0},
                                            {u.z,v.z,w.z,0},
                                            {0,0,0,1}};
            /* rMatrix is rotation along X axis since now u is aligned with X*/
            double rMatrix[4][4] = {{1,0,0,0},
                                    {0,cos(r->angle * M_PI/180),(-1) * sin(r->angle * M_PI/180),0},
                                    {0,sin(r->angle * M_PI/180),cos(r->angle * M_PI/180),0},
                                    {0,0,0,1}};
            Matrix4 rot1 = multiplyMatrixWithMatrix(rMatrix, mMatrix);
            Matrix4 rotRes = multiplyMatrixWithMatrix(mMatrix_inverse, rot1);
            M_model = multiplyMatrixWithMatrix(rotRes, M_model);
        }
        else {
            fprintf(stderr, "Invalid Transformation Type!\n");
        }
    }
    return M_model;
}

Matrix4 calcCameraTransMatrix(Camera * camera) {
    double T[4][4] = {{1, 0, 0, -(camera->pos.x)},
                        {0, 1, 0, -(camera->pos.y)},
                        {0, 0, 1, -(camera->pos.z)},
                        {0, 0, 0, 1}};
    double R[4][4] = {{camera->u.x, camera->u.y, camera->u.z, 0},
                      {camera->v.x, camera->v.y, camera->v.z, 0},
                      {camera->w.x, camera->w.y, camera->w.z, 0},
                      {0, 0, 0, 1}};
    return multiplyMatrixWithMatrix(R, T);
}

Matrix4 calcProjectionTransMatrix(Camera * camera, bool projType) {
    if (projType) {
        /* Return M_pers */
        double M_pers[4][4] = {{(2*camera->near) / (camera->right - camera->left), 0, (camera->right + camera->left) / (camera->right - camera->left), 0},
                               {0, (2*camera->near) / (camera->top - camera->bottom), (camera->top + camera->bottom) / (camera->top - camera->bottom), 0},
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

Matrix4 calcViewportTransMatrix(Camera * camera) {
    double M_viewport[4][4] = {{camera->horRes/2.0, 0, 0, (camera->horRes-1)/2.0},
                               {0, camera->verRes/2.0, 0, (camera->verRes-1)/2.0},
                               {0, 0, 0.5, 0.5},
                               {0, 0, 0, 1}}; // WARNING: We do not need last line semantically but need in code!
    return Matrix4(M_viewport);
}

bool isBackfaceCulled(Vec4 & v0, Vec4 & v1, Vec4 & v2) {
    Vec3 v_0 = Vec3(v0.x, v0.y, v0.z, v0.colorId);
    Vec3 v_1 = Vec3(v1.x, v1.y, v1.z, v1.colorId);
    Vec3 v_2 = Vec3(v2.x, v2.y, v2.z, v2.colorId);
    Vec3 edge01 = subtractVec3(v_1, v_0);
    Vec3 edge02 = subtractVec3(v_2, v_0);
    Vec3 normalVector = normalizeVec3(crossProductVec3(edge01, edge02));
    double res = dotProductVec3(normalVector, v_0); // View Vector = v_0 - origin
    return (res < 0);
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

bool clipLine(std::pair<Vec4, Color> & pair1, std::pair<Vec4, Color> & pair2) {
    /* Clips given line with Liang-Barsky Algorithm in 3D
     * as a result v0 and v1 gets updated as needed */
    bool isVisible = false;
    Vec4 v0 = pair1.first, v1 = pair2.first;
    Color c0 = pair1.second, c1 = pair2.second;
    double t_E = 0, t_L = 1;
    double dx = v1.x - v0.x, dy = v1.y - v0.y, dz = v1.z - v0.z;
    Color dc = c1 - c0;
    double x_min = -1, y_min = -1, z_min = -1;

    double x_max = 1, y_max = 1, z_max = 1;
    if (visible(dx, x_min-v0.x, t_E, t_L) && visible(-dx, v0.x-x_max, t_E, t_L)
        && visible(dy, y_min-v0.y, t_E, t_L) && visible(-dy, v0.y-y_max, t_E, t_L)
        && visible(dz, z_min-v0.z, t_E, t_L) && visible(-dz, v0.z-z_max, t_E, t_L)) {
        isVisible = true;
        /* Check if at least some part of the line is clipped */
        if (t_L < 1) {
            v1.x = v0.x + (dx * t_L);
            v1.y = v0.y + (dy * t_L);
            v1.z = v0.z + (dz * t_L);
            c1 = c0 + (dc * t_L);
        }
        if (t_E > 0) {
            v0.x = v0.x + (dx * t_E);
            v0.y = v0.y + (dy * t_E);
            v0.z = v0.z + (dz * t_E);
            c0 = c0 + (dc * t_E);
        }
    }

    pair1.first = v0;
    pair1.second = c0;
    pair2.first = v1;
    pair2.second = c1;

    return isVisible;
}

void rasterizeLine(vector<vector<Color>> & image, Vec4 & v0, Vec4 & v1, Color & c0, Color & c1) {
    double dx = v1.x - v0.x;
    double dy = v1.y - v0.y;
    int d, incrAmount = 1;
    Color dc, c;

    /* First Check if the slope is between 0 < m <= 1 */
    if (abs(dy) <= abs(dx)) {
        /* Normal Midpoint Algorithm */
        if (v1.x < v0.x) {
            swap(v0, v1);
            swap(c0, c1);
        }
        if (v1.y < v0.y) {
            /* Make sure that line goes in negative direction in each iteration */
            incrAmount = -1;
        }

        int y = v0.y;
        c = c0;
        d = (v0.y - v1.y) + (incrAmount * 0.5 * (v1.x - v0.x));
        dc = (c1 - c0) / (v1.x - v0.x);
        for (int x = v0.x; x <= v1.x; x++) {
            image[x][y] = c.round();
            if (d * incrAmount < 0) { // choose NE
                y += incrAmount;
                d += (v0.y - v1.y) + (incrAmount * (v1.x - v0.x));
            }
            else // choose E
                d += (v0.y - v1.y);
            c = c + dc;
        }
    }
    else if (abs(dy) > abs(dx)) {
        /* Modified Midpoint Algorithm for 1 < m < INF */
        if (v1.y < v0.y) {
            swap(v0, v1);
            swap(c0, c1);
        }
        if (v1.x < v0.x) {
            /* Make sure that line goes in negative direction in each iteration */
            incrAmount = -1;
        }

        int x = v0.x;
        c = c0;
        d = (v1.x - v0.x) + (incrAmount * 0.5 * (v0.y - v1.y));
        dc = (c1 - c0) / (v1.y - v0.y);

        for (int y = v0.y; y <= v1.y; y++) {
            image[x][y] = c.round();
            if (d * incrAmount > 0) {
                x += incrAmount;
                d += (v1.x - v0.x) + (incrAmount * (v0.y - v1.y));
            }
            else
                d += (v1.x - v0.x);
            c = c + dc;
        }
    }
}

double f_(double x, double y, double x_n, double y_n, double x_m, double y_m){
    return (x * (y_n - y_m)) + (y * (x_m - x_n)) + (x_n * y_m) - (y_n * x_m);
}

void rasterizeTriangle(vector<vector<Color>> & image, const Color * c0, const Color * c1, const Color * c2,
        Vec4 & v0, Vec4 & v1, Vec4 & v2, int nx, int ny) {

    int x_min = min(min(v0.x, v1.x), v2.x) >= 0 ? min(min(v0.x, v1.x), v2.x) : 0;
    x_min = x_min <= nx-1 ? x_min: nx-1;
    int y_min = min(min(v0.y, v1.y), v2.y) >= 0 ? min(min(v0.y, v1.y), v2.y) : 0;
    y_min = y_min <= ny-1 ? y_min: ny-1;

    int x_max = max(max(v0.x, v1.x), v2.x) < 0 ? 0 : max(max(v0.x, v1.x), v2.x) > nx-1 ? nx-1 : max(max(v0.x, v1.x), v2.x);
    int y_max = max(max(v0.y, v1.y), v2.y) < 0 ? 0 : max(max(v0.y, v1.y), v2.y) > ny-1 ? ny-1 : max(max(v0.y, v1.y), v2.y);

    double alpha,beta,gamma;
    Color c;

    for(int y=y_min; y<=y_max; ++y){
        for(int x=x_min; x<=x_max; ++x){
            alpha = f_(x,y, v1.x, v1.y, v2.x, v2.y) / f_(v0.x,v0.y, v1.x,v1.y, v2.x,v2.y); // L12
            beta = f_(x,y, v2.x, v2.y, v0.x, v0.y) / f_(v1.x,v1.y, v2.x,v2.y, v0.x,v0.y); // L20
            gamma = f_(x,y, v0.x, v0.y, v1.x, v1.y) / f_(v2.x,v2.y, v0.x,v0.y, v1.x,v1.y); // L01

            if(alpha>=0 && beta>=0 && gamma>=0){
                c = ((*c0) * alpha) + ((*c1) * beta) + ((*c2) * gamma);
                image[x][y] = c.round();
            }
        }
    }
}


/*
	Transformations, clipping, culling, rasterization are done here.
	You can define helper functions inside Scene class implementation.
*/
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
        - Line Rasterization => Midpoint Algorithm => BE CAREFUL WITH SLOPES!
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
    Matrix4 M_cam = calcCameraTransMatrix(camera);
    Matrix4 M_proj = calcProjectionTransMatrix(camera, this->projectionType);
    Matrix4 M_viewport = calcViewportTransMatrix(camera);

    /* For each model apply these transformations + clip + cull then rasterize */
    for (const auto & model : this->models) {
        Matrix4 M_model = calcModelingTransMatrix(camera, *model, this->translations, this->rotations, this->scalings);
        Matrix4 M_cam_model = multiplyMatrixWithMatrix(M_cam, M_model); // Mcam * Mmodel
        Matrix4 M_proj_cam_model = multiplyMatrixWithMatrix(M_proj, M_cam_model); // Mproj * Mcam * Mmodel
        for (auto & triangle : model->triangles) {
            const Vec3 * v0 = this->vertices[triangle.getFirstVertexId()-1];
            const Vec3 * v1 = this->vertices[triangle.getSecondVertexId()-1];
            const Vec3 * v2 = this->vertices[triangle.getThirdVertexId()-1];
            const Color * c0 = this->colorsOfVertices[v0->colorId-1];
            const Color * c1 = this->colorsOfVertices[v1->colorId-1];
            const Color * c2 = this->colorsOfVertices[v2->colorId-1];

            Vec4 projectedV0 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v0->x, v0->y, v0->z, 1, v0->colorId));
            Vec4 projectedV1 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v1->x, v1->y, v1->z, 1, v1->colorId));
            Vec4 projectedV2 = multiplyMatrixWithVec4(M_proj_cam_model, Vec4(v2->x, v2->y, v2->z, 1, v2->colorId));

            /* Culling Phase if enabled */
            if (this->cullingEnabled && isBackfaceCulled(projectedV0, projectedV1, projectedV2)) {
                /* If backface culling is enabled and model is backfacing then just continue, do not render this model */
                continue;
            }

            if (!model->type) {
                /* Wireframe mode */

                /* Perform Perspective Division */
                /* Construct Lines to be clipped
                 * For each line create 2 pairs<Vec4, Color> which represent the initial and final points of this line
                 * Pair's Vec4 holds the point of the vertice - can be updated during clipping
                 * Pair's Color holds the color of the vertice - can be updated during clipping
                 * */

                // Line-1 => L01
                std::pair <Vec4, Color> L01_pair1 = std::make_pair(projectedV0, *c0);
                std::pair <Vec4, Color> L01_pair2 = std::make_pair(projectedV1, *c1);
                L01_pair1.first /= L01_pair1.first.t;
                L01_pair2.first /= L01_pair2.first.t;

                // Line-2 => L12
                std::pair <Vec4, Color> L12_pair1 = std::make_pair(projectedV1, *c1);
                std::pair <Vec4, Color> L12_pair2 = std::make_pair(projectedV2, *c2);
                L12_pair1.first /= L12_pair1.first.t;
                L12_pair2.first /= L12_pair2.first.t;

                // Line-3 => L20
                std::pair <Vec4, Color> L20_pair1 = std::make_pair(projectedV2, *c2);
                std::pair <Vec4, Color> L20_pair2 = std::make_pair(projectedV0, *c0);
                L20_pair1.first /= L20_pair1.first.t;
                L20_pair2.first /= L20_pair2.first.t;

                /* Clipping Phase */
                bool L01_visibility = clipLine(L01_pair1, L01_pair2);
                bool L12_visibility = clipLine(L12_pair1, L12_pair2);
                bool L20_visibility = clipLine(L20_pair1, L20_pair2);

                /* Viewport Transformation Phase */
                // L01
                L01_pair1.first = multiplyMatrixWithVec4(M_viewport, L01_pair1.first);
                L01_pair2.first = multiplyMatrixWithVec4(M_viewport, L01_pair2.first);

                // L12
                L12_pair1.first = multiplyMatrixWithVec4(M_viewport, L12_pair1.first);
                L12_pair2.first = multiplyMatrixWithVec4(M_viewport, L12_pair2.first);

                // L20
                L20_pair1.first = multiplyMatrixWithVec4(M_viewport, L20_pair1.first);
                L20_pair2.first = multiplyMatrixWithVec4(M_viewport, L20_pair2.first);

                /* Final step of FRP - Rasterize the Line and fill the image for this model */
                if(L01_visibility)
                    rasterizeLine(this->image, L01_pair1.first, L01_pair2.first, L01_pair1.second, L01_pair2.second); // L01
                if(L12_visibility)
                    rasterizeLine(this->image, L12_pair1.first, L12_pair2.first, L12_pair1.second, L12_pair2.second); // L12
                if(L20_visibility)
                    rasterizeLine(this->image, L20_pair1.first, L20_pair2.first, L20_pair1.second, L20_pair2.second); // L20
            }
            else {
                /* Solid mode */

                /* Perform Perspective Division */
                projectedV0 /= projectedV0.t;
                projectedV1 /= projectedV1.t;
                projectedV2 /= projectedV2.t;

                /* Viewport Transformation Phase */
                Vec4 viewportV0 = multiplyMatrixWithVec4(M_viewport, projectedV0);
                Vec4 viewportV1 = multiplyMatrixWithVec4(M_viewport, projectedV1);
                Vec4 viewportV2 = multiplyMatrixWithVec4(M_viewport, projectedV2);

                /* Final step of FRP - Rasterize the Triangle and fill the image for this model */
                rasterizeTriangle(this->image, c0, c1, c2, viewportV0, viewportV1, viewportV2, camera->horRes, camera->verRes);
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