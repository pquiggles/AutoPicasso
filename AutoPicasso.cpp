/*
 * File: AutoPicasso.cpp
 * ----------------------
 * Name: Paul Quigley
 * Section: Lingtong Sun
 * This file is the starter project for the random writer problem
 * on Assignment #2.
 *
 * This program draws a landscape painting using seven different recursive functions.
 *
 * The user can select a slew of features he wants to be in the landscape painting, and the program will draw it
 * automatically. All drawings make use of stochastic processes. Every drawing will be different.
 *
 * I encourage users with access to the source to play around with the constants. See what happens when you up the terrain
 * order to 6, for examples.
 *
 * This code was made in XCode, and I did not have a chance to test it in Visual Studio, so if it crashes in Visual Studio,
 * I apologize, and encourage such section leaders to try it  in XCode when they have a chance.
 */

#include <iostream>
#include "gwindow.h"
#include "gobjects.h"
#include "console.h"
#include "simpio.h"
#include "math.h"
#include "random.h"
#include "strlib.h"
#include "grid.h"
#include "map.h"
#include "ginteractors.h"
#include "gevents.h"
using namespace std;


/*Window Constants*/
const double width = 600;
const double height = 600;

/*Sun Constants*/
const int MAX_SUN_RADIUS = 70;
const int DUSK_SUN_COLOR = 16723967;
const int DAY_SUN_COLOR = 16777190;

/*Sky Constants*/
const int SKY_ORDER = 6;
const int RED = 65536;
const int DUSK_BASE = 45535;
const int DAY_BASE = 65535;
const int WHITE=65535;
const double STAR_WIDTH = .1; //Should be between zero and one.

/*Terrain Constants*/
const int TERRAIN_ORDER = 5;
const double TERRAIN_DEFORMATION = .3; //This deformation should be between zero and one, as should all.
const int TERRAIN_BASE_COLOR = 5597969;

/*Mountain Constants*/
const double BACK_MOUNTAIN_DEFORMATION=.6;
const int BACK_MOUNTAIN_ORDER=8;
const int MOUNTAIN_DAY_COLOR=2640896;
const int MOUNTAIN_LATE_COLOR=4605769;

/*Lightning Constants*/
const int LIGHTNING_ORDER = 7;
const int LIGHTNING_WIDTH=5;
const double LIGHTNING_DEFORMATION=.7;
const int LIGHTNING_OUTER_COLOR = 6157311;
const int LIGHTNING_INNER_COLOR = 15660031;

/*Tree Constants*/
const int TREE_ORDER = 5;
const int BRANCH_THICKNESS = 15;
const double TREE_DEFORMATION=.2;

/*Instance Variables (sue me)*/
Map<double, GPoint> closePoints;
bool dusk = true;
bool day = false;
bool night = false;

/*Sky Methods*/
void drawSky(GWindow gw);
void skyColoring(Grid<int> & colors, GWindow gw);
int sdAlgo(Grid<int> & grid, int order, int xMin, int xMax, int yMin, int yMax, int randMax); //Recursive

/*Sun Methods*/
int drawSun(GWindow gw, int radius, int color); //Recursive

/*Terrain Methods*/
void drawTerrain(GWindow gw);
int foldedTriangle(int order, GPoint triangle[], GWindow gw); //Recursive
void drawTriangle(GPoint triangle[], GWindow gw);
GPoint randomClosePoint(GPoint a, GPoint b, int order);

/*Mountain Methods*/
void drawBackGroundMountain(GWindow gw);
GPolygon * mountainPolygon();
int mountainPoints(Vector<GPoint> & points, int order); //Recursive
GPoint verticallyDisplacedPoint(GPoint a, GPoint b, int order);

/*Lightning Methods*/
void drawLightning(GWindow gw, GPoint start, GPoint end);
GPolygon * lightningPolygon(GPoint start, GPoint finish); //Recursive
int hDisplacedPoints(Vector<GPoint> & points, int order, double deformation);
GPoint horizontallyDisplacedPoint(GPoint a, GPoint b, int order, double deformation); //Recursive


/*Tree Methods*/
void drawTree(GWindow gw);
void drawTrunk(GWindow gw, GPoint start);
void drawBranch(GWindow gw, GPoint start, GPoint finish, double width); //Recursive
GPolygon * branchPolygon(GPoint start, GPoint finish, double width);
GPolygon * trunkPolygon(GPoint start, GPoint finish, double width);

int main() {
	GWindow gw(width, height);
    
     while(true){
         
         /*Button Setup*/
         
         GButton *button = new GButton("Draw");
         gw.addToRegion(button, "South");
    
         GChooser *timeChooser = new GChooser();
         timeChooser->addItem("Dusk");
         timeChooser->addItem("Day");
         timeChooser->addItem("Night");
         gw.addToRegion(timeChooser, "South");
    
         GCheckBox *fg = new GCheckBox("Terrain");
         gw.addToRegion(fg, "West");
    
         GCheckBox *sun = new GCheckBox("Sun/Moon");
         gw.addToRegion(sun, "West");
    
         GCheckBox *bg = new GCheckBox("Mountain");
         gw.addToRegion(bg, "West");
    
    
         GCheckBox *lightning = new GCheckBox("Lightning");
         gw.addToRegion(lightning, "West");
         
         GCheckBox *tree = new GCheckBox("Tree");
         gw.addToRegion(tree, "West");
    
        /*Listen for click*/
         while(true){
            GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
            if(e.getEventClass() == ACTION_EVENT) break;
        }
        string selectedTime = timeChooser->getSelectedItem();
         if(selectedTime=="Dusk"){
             dusk=true;
             day=false;
             night=false;
         }
         if(selectedTime=="Day"){
            day=true;
            dusk=false;
            night=false;
        }
        if(selectedTime=="Night"){
            night=true;
            dusk=false;
            day=false;
        }
         /*Drawing selected items*/
        gw.clear();
        closePoints.clear();
        drawSky(gw);
        if(sun->isSelected()) drawSun(gw,MAX_SUN_RADIUS,(dusk?DUSK_SUN_COLOR:DAY_SUN_COLOR));
        if(bg->isSelected()) drawBackGroundMountain(gw);
        if(lightning->isSelected()) drawLightning(gw, GPoint(width*randomReal(.2, .8), 0), GPoint(width*randomReal(.2, .8), height));
        if(tree->isSelected()) drawTree(gw);
        if(fg->isSelected()) drawTerrain(gw);
        }

	return 0;
}

/*
 * Function: drawSky
 * Usage: drawSky(gw);
 * --------------------------
 * Initializes a grid of sky sky colors using the square diamond algorithm.
 * Calls function to color in the squares at the appropriate places on the graphics window.
 */
void drawSky(GWindow gw){
    int iMax = pow(2,SKY_ORDER);
    Grid<int> colors(iMax+1,iMax+1);
    colors[0][0]=0;
    
    /* The max value for any RGB element is 255.
     * Setting the corner's colors to 255/2 with a randMax of 255/2 makes the whole color spectrum possible.
     * Yet any squares value will never exceed 255.
     */
    colors[0][iMax]=255/2;
    colors[iMax][0]=255/2;
    colors[iMax][iMax]=0;
    int xMin = 0;
    int xMax = iMax;
    int yMin = 0;
    int yMax = iMax;
    int randMax = 255/2;
    sdAlgo(colors, SKY_ORDER, xMin, xMax, yMin, yMax, randMax);
    skyColoring(colors,gw);
}


/*
 * Function: sdAlgo
 * Usage: sdAlgo(squareDiamond, SKY_ORDER, xMin, xMax, yMin, yMax, randMax);
 * --------------------------
 * Implementation of the square diamond algorithm.
 * Produces a grid of integers whose magnitudes are "cloudy".
 * This website has a better description of the algorithm: http://www.gameprogrammer.com/fractal.html
 */
int sdAlgo(Grid<int> & grid, int order, int xMin, int xMax, int yMin, int yMax, int randMax){
    if(order==0){
        return 0;
    }
    int xAvg = (xMin+xMax)/2;
    int yAvg = (yMin+yMax)/2;
    int valAvg = grid[xMin][yMin]+grid[xMin][yMax]+grid[xMax][yMin]+grid[xMax][yMax];
    valAvg /= 4;
    grid[xAvg][yAvg] = valAvg+randomInteger(0, randMax);
    grid[xMin][yAvg] = (grid[xMin][yMin]+grid[xMin][yMax])/2+randomInteger(0, randMax);
    grid[xAvg][yMin] = (grid[xMin][yMin]+grid[xMax][yMin])/2+randomInteger(0, randMax);
    grid[xMax][yAvg] = (grid[xMax][yMin]+grid[xMax][yMax])/2+randomInteger(0, randMax);
    grid[xAvg][yMax] = (grid[xMin][yMax]+grid[xMax][yMax])/2+randomInteger(0, randMax);
    randMax /=2;
    return sdAlgo(grid, order-1, xMin, xAvg, yMin, yAvg, randMax)+sdAlgo(grid, order-1, xAvg, xMax, yMin, yAvg, randMax)+sdAlgo(grid, order-1, xMin, xAvg, yAvg, yMax, randMax)+sdAlgo(grid, order-1, xAvg, xMax, yAvg, yMax, randMax);
}

/*
 * Function: skyColoring
 * Usage: skyColoring(colors,gw);
 * --------------------------
 * Iterates over the graphics window and adds squares of the appropriate color from the sdAlgo grid.
 * Randomly adds stars if night is set to true.
 */
void skyColoring(Grid<int> & colors, GWindow gw){
    double xStep = (double)width/colors.numCols();
    double yStep = (double)height/colors.numRows();
    int starInfreq = 14;
    for(int j=0; j<colors.numRows(); j++){
        for(int i=0; i<colors.numCols(); i++){
            int color = colors[i][j]*RED+(dusk?DUSK_BASE:DAY_BASE);
            if(night){
                color = colors[i][j]*.5;
            }
            GRect *rect = new GRect(xStep*j, yStep*i, xStep, yStep);
            rect->setFilled(true);
            rect->setColor(color);
            gw.add(rect);
            if(night){
                if(rand()%starInfreq==1){
                    GRect *star = new GRect(xStep*j, yStep*i, xStep*STAR_WIDTH, yStep*STAR_WIDTH);
                    star->setFilled(true);
                    star->setColor(WHITE);
                    gw.add(star);
                }
            }
        }
        starInfreq++;
    }
}

/*
 * Function: drawSun
 * Usage: if(sun->isSelected()) drawSun(gw,MAX_SUN_RADIUS,(dusk?DUSK_SUN_COLOR:DAY_SUN_COLOR));
 * --------------------------
 * Uses a recursive function to draw the sun/moon as series of concentric circles.
 * Totally unnecessary to do this recursively, but it's the recursive contest. #YOLO
 */
int drawSun(GWindow gw, int radius, int color){
    if(radius<=0){
        return 0;
    }
    GOval *oval;
    if(dusk){
        oval = new GOval(width/4-radius,height*.45-radius,radius*2,radius*2);
    }
    else{
        radius/=2;
        oval = new GOval(2.0*width/3.0-radius,height/3.0-radius,radius*2,radius*2);
        radius*=2;
    }
    oval->setColor(color);
    oval->setFilled(true);
    oval->setColor(color);
    gw.add(oval);
    radius--;
    color -=2;
    return drawSun(gw, radius, color);
}


/*
 * Function: drawTerrain
 * Usage: if(fg->isSelected()) drawTerrain(gw)
 * --------------------------
 * Draws two folded triangles in foreground that sort of look like 3D terrain.
 */
void drawTerrain(GWindow gw){
    GPoint leftTri[3];
    leftTri[0] = GPoint(-50,height*randomReal(.5, .7));
    leftTri[1] = GPoint(-50,height+50);
    leftTri[2] = GPoint(width+50,height+50);
    foldedTriangle(TERRAIN_ORDER, leftTri, gw);
    closePoints.clear();
    GPoint rightTri[3];
    rightTri[0] = GPoint(width+50,height*randomReal(.5, .7));
    rightTri[1] = GPoint(width+50,height+50);
    rightTri[2] = GPoint(-50,height+50);
    foldedTriangle(TERRAIN_ORDER, rightTri, gw);
}


/*
 * Function: foldedTriangle
 * Usage: foldedTriangle(TERRAIN_ORDER, leftTri, gw);
 * --------------------------
 * Basically Sierpinski's triangle, but it also moves the midpoint of each triangle with each iteration.
 */
int foldedTriangle(int order, GPoint triangle[], GWindow gw){
    if(order == 0){
        drawTriangle(triangle, gw);
        return 0;
    }
    GPoint A = randomClosePoint(triangle[0], triangle[1], order);
    GPoint B = randomClosePoint(triangle[0], triangle[2], order);
    GPoint C = randomClosePoint(triangle[1], triangle[2], order);
    GPoint triangle1[3] = {triangle[0], A, B};
    GPoint triangle2[3] = {triangle[1], A, C};
    GPoint triangle3[3] = {triangle[2],B, C};
    GPoint triangle4[3] = {A,B,C};
    return foldedTriangle(order-1, triangle1, gw) + foldedTriangle(order-1, triangle2, gw)+foldedTriangle(order-1, triangle3, gw)+foldedTriangle(order-1, triangle4, gw);
}

/*
 * Function: drawTriangle
 * Usage: drawTriangle(triangle, gw);
 * --------------------------
 * Given an array of GPoints, this function draws a triangle.
 */
void drawTriangle(GPoint triangle[], GWindow gw){
    GPolygon *tri = new GPolygon();
    for(int i=0; i<3; i++){
        tri->addVertex(triangle[i].getX(), triangle[i].getY());
    }
    tri->setFilled(true);
    tri->setFillColor(TERRAIN_BASE_COLOR+rand()%100);
    gw.add(tri);
}

/*
 * Function: randomClosePoint
 * Usage:  GPoint A = randomClosePoint(triangle[0], triangle[1], order);
 * --------------------------
 * Given two points, this algorithm finds a point near their midpoint, but displaced by a random amount.
 */
GPoint randomClosePoint(GPoint a, GPoint b, int order){
   /* With the folded triangle algorithm, you want to return the same randomClosePoint every time.
    * This is basically a checksum so that the algorithm does so, but can maintain a high degree of randomness,
    * which setting the same random seed over and over again won't do.
    */
    double key = a.getX()+a.getY()+b.getX()+b.getY();
    if(closePoints.containsKey(key)){
        return closePoints[key];
    }
    double xAve = (a.getX()+b.getX())/2;
    double yAve = (a.getY()+b.getY())/2;
    double deform = TERRAIN_DEFORMATION/exp(TERRAIN_ORDER-order);
    double x = randomReal((1-deform)*xAve, (1+deform)*xAve);
    double y = randomReal((1-deform)*yAve, (1+deform)*yAve);
    closePoints.put(key, GPoint(x,y));
    return GPoint(x, y);
}

/*
 * Function: drawBackGroundMountain
 * Usage:  if(bg->isSelected()) drawBackGroundMountain(gw);
 * --------------------------
 * Adds a mountain to the to the GWindow, and sets it's color depending on the time of day.
 */
void drawBackGroundMountain(GWindow gw){
    GPolygon *mountain = mountainPolygon();
    mountain->setFilled(true);
    mountain->setColor(MOUNTAIN_DAY_COLOR);
    if(!day) mountain->setColor(MOUNTAIN_LATE_COLOR);
    gw.add(mountain);
}

/*
 * Function: mountainPolygon
 * Usage:  GPolygon *mountain = mountainPolygon();
 * --------------------------
 * Returns a mountain-like polygon consting of a set of vertically displaced points, and the corners of the screen.
 */
GPolygon * mountainPolygon(){
    Vector<GPoint> points;
    points.add(GPoint(0, height/2));
    points.add(GPoint(width, height/2));
    mountainPoints(points, BACK_MOUNTAIN_ORDER);
    GPolygon *mountain = new GPolygon();
    mountain->addVertex(0, height);
    for(int i=0; i<points.size(); i++){
        mountain->addVertex(points[i].getX(), points[i].getY());
    }
    mountain->addVertex(width, height);
    return mountain;
}

/*
 * Function: mountainPoints
 * Usage:  mountainPoints(points, BACK_MOUNTAIN_ORDER);
 * --------------------------
 * Inserts a vertically displaced point between every point in the vector of points.
 */
int mountainPoints(Vector<GPoint> & points, int order){
    if(order == 0){
        return 0;
    }
    int originalSize = points.size();
    for(int i = 0; i<originalSize-1; i++){
        points.insert(2*i+1, verticallyDisplacedPoint(points[2*i], points[2*i+1], order));
    }
    return mountainPoints(points, order-1);
}

/*
 * Function: verticallyDisplacedPoint
 * Usage: points.insert(2*i+1, verticallyDisplacedPoint(points[2*i], points[2*i+1], order));
 * --------------------------
 * Like randomClosePoint, but only displaced the point in the y direction.
 */
GPoint verticallyDisplacedPoint(GPoint a, GPoint b, int order){
    double xAve = (a.getX()+b.getX())/2;
    double yAve = (a.getY()+b.getY())/2;
    double deform = BACK_MOUNTAIN_DEFORMATION/exp(BACK_MOUNTAIN_ORDER-order);
    double y = randomReal((1-deform)*yAve, (1+deform)*yAve);
    return GPoint(xAve,y);
}

/*
 * Function: horizontallyDisplacedPoint
 * Usage: points.insert(2*i+1, horizontallyDisplacedPoint(points[2*i], points[2*i+1], order));
 * --------------------------
 * Like randomClosePoint, but only displaced the point in the x direction.
 */
GPoint horizontallyDisplacedPoint(GPoint a, GPoint b, int order, double deformation){
    double xAve = (a.getX()+b.getX())/2;
    double yAve = (a.getY()+b.getY())/2;
    double deform = deformation/exp(LIGHTNING_ORDER-order);
    double x = randomReal((1-deform)*xAve, (1+deform)*xAve);
    return GPoint(x,yAve);
}

/*
 * Function: drawLightning
 * Usage: if(lightning->isSelected()) drawLightning(gw, GPoint(width*randomReal(.2, .8), 0), GPoint(width*randomReal(.2, .8), height));
 * --------------------------
 * Adds a lightning shaped polygon to the screen that branches off probabilistically.
 */
void drawLightning(GWindow gw, GPoint start, GPoint end){
    GPolygon *lightning = lightningPolygon(start, end);
    lightning->setFilled(true);
    lightning->setColor(LIGHTNING_OUTER_COLOR);
    lightning->setFillColor(LIGHTNING_INNER_COLOR);
    gw.add(lightning);
    if(randomChance(.7)){
        Vector<GPoint> points = lightning->getVertices();
        start = points[points.size()/4];
        drawLightning(gw, start,GPoint(start.getX(), height));
    }
}

/*
 * Function: lightningPolygon
 * Usage: GPolygon *lightning = lightningPolygon(start, end);
 * --------------------------
 * Returns a lightning shaped polygon going from start to end.
 */
GPolygon * lightningPolygon(GPoint start, GPoint finish){
    Vector<GPoint> points;
    points.add(start);
    points.add(finish);
    hDisplacedPoints(points, LIGHTNING_ORDER, LIGHTNING_DEFORMATION);
    GPolygon *lightning = new GPolygon();
    for(int i=0; i<points.size(); i++){
        lightning->addVertex(points[i].getX(), points[i].getY());
    }
    for(int i=points.size()-1; i>=0; i--){
        lightning->addVertex(points[i].getX()+LIGHTNING_WIDTH, points[i].getY());
    }
    return lightning;
}

/*
 * Function: hDisplacedPoints
 * Usage: Both in lightning algorithm and tree algorithm
 * --------------------------
 * Inserts a horizontally displaced point between each point in the vector of points.
 */
int hDisplacedPoints(Vector<GPoint> & points, int order, double deformation){
    if(order == 0){
        return 0;
    }
    int originalSize = points.size();
    for(int i = 0; i<originalSize-1; i++){
        points.insert(2*i+1, horizontallyDisplacedPoint(points[2*i], points[2*i+1], order, deformation));
    }
    return hDisplacedPoints(points, order-1, deformation);
}

/*
 * Function: drawTree
 * Usage: drawTree(gw);
 * --------------------------
 * Draws a tree on the GWindow.
 */
void drawTree(GWindow gw){
    double xMult = randomReal(.4, .6);
    double yMult = randomReal(.5, .7);
    GPoint branchStart = GPoint(width*xMult,height*yMult);
    drawTrunk(gw, branchStart);
    drawBranch(gw, branchStart, GPoint(width*(xMult-.15), height*(yMult-.15)), BRANCH_THICKNESS);
    drawBranch(gw, branchStart, GPoint(width*(xMult), height*(yMult-.15)), BRANCH_THICKNESS);
    drawBranch(gw, branchStart, GPoint(width*(xMult+.15), height*(yMult-.15)), BRANCH_THICKNESS);
}

/*
 * Function: drawTrunk
 * Usage: drawTrunk(gw, branchStart);;
 * --------------------------
 * Draws a curvy looking trunk structure on the GWindow.
 */
void drawTrunk(GWindow gw, GPoint start){
    GPoint finish = GPoint(start.getX(), height);
    GPolygon * trunk = trunkPolygon(start, finish, BRANCH_THICKNESS);
    trunk->setFilled(true);
    trunk->setColor(4270639);
    gw.add(trunk);
}

/*
 * Function: drawBranch
 * Usage: drawBranch(gw, branchStart, GPoint(width*(xMult-.15), height*(yMult-.15)), BRANCH_THICKNESS);
 * --------------------------
 * Draws branches that continue and branch of probabilistically.
 */
void drawBranch(GWindow gw, GPoint start, GPoint finish, double width){
    GPolygon *branch = branchPolygon(start, finish, width);
    branch->setFilled(true);
    branch->setColor(4270639);
    gw.add(branch);
    Vector<GPoint> points = branch->getVertices();
    start = points[points.size()/4];
    double endX = randomReal(start.getX()*(1-TREE_DEFORMATION), start.getX()*(1+TREE_DEFORMATION));
    double endY = randomReal(1+TREE_DEFORMATION, 1+2*TREE_DEFORMATION)*(finish.getY()-start.getY())+start.getY();
    if((endY-finish.getY())<-3){
        drawBranch(gw, start,GPoint(endX, endY),width/2);
    }
}

/*
 * Function: branchPolygon
 * Usage: GPolygon *branch = branchPolygon(start, finish, width);
 * --------------------------
 * Draws a branch shaped polygon that tapers as it's height increases.
 */
GPolygon * branchPolygon(GPoint start, GPoint finish, double width){
    Vector<GPoint> points;
    points.add(start);
    points.add(finish);
    hDisplacedPoints(points, TREE_ORDER, TREE_DEFORMATION);
    GPolygon *branch = new GPolygon();
    for(int i=0; i<points.size(); i++){
        branch->addVertex(points[i].getX(), points[i].getY());
    }
    for(int i=points.size()-1; i>=0; i--){
        double thickness = width*(points[i].getY()-finish.getY())/(start.getY()-finish.getY());
        branch->addVertex(points[i].getX()+thickness, points[i].getY());
    }
    return branch;
}

/*
 * Function: trunkPolygon
 * Usage: GPolygon * trunk = trunkPolygon(start, finish, BRANCH_THICKNESS);
 * --------------------------
 * Returns a curvy trunk shaped polygon that decreases in width as it gets to the branching point.
 */
GPolygon * trunkPolygon(GPoint start, GPoint finish, double width){
    Vector<GPoint> points;
    points.add(start);
    points.add(finish);
    hDisplacedPoints(points, TREE_ORDER, TREE_DEFORMATION);
    GPolygon *branch = new GPolygon();
    for(int i=0; i<points.size(); i++){
        branch->addVertex(points[i].getX(), points[i].getY());
    }
    for(int i=points.size()-1; i>=0; i--){
        double thickness =  (1+(points[i].getY()-start.getY())/(finish.getY()-start.getY()))*width;
        branch->addVertex(points[i].getX()+thickness, points[i].getY());
    }
    return branch;
}




