/*
 * File: RandomWriter.cpp
 * ----------------------
 * Name: [TODO: enter name here]
 * Section: [TODO: enter section leader here]
 * This file is the starter project for the random writer problem
 * on Assignment #2.
 * [TODO: extend the documentation]
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

const int MAX_SUN_RADIUS = 70;

const double width = 600;
const double height = 500;

const int SKY_ORDER = 6;
const int TERRAIN_ORDER = 6;
const double TERRAIN_DEFORMATION = .3;
const double BACK_MOUNTAIN_DEFORMATION=.6;
const int BACK_MOUNTAIN_ORDER=8;
const int MOUNTAIN_COLOR_ORDER=7;

Map<double, GPoint> closePoints;
bool dusk = true;
bool day = false;
bool night = false;
bool foreground = true;

void drawSky(GWindow gw);
void skyColoring(Grid<int> & colors, GWindow gw);
int drawSun(GWindow gw, int radius, int color);
void drawTerrain(GWindow gw);
int foldedTriangle(int order, GPoint triangle[], GWindow gw);
void drawTriangle(GPoint triangle[], GWindow gw);
int sdAlgo(Grid<int> & grid, int order, int xMin, int xMax, int yMin, int yMax, int randMax);
GPoint randomClosePoint(GPoint a, GPoint b, int order);
void drawBackGroundMountain(GWindow gw);
GPolygon * mountainPolygon();
int mountainPoints(Vector<GPoint> & points, int order);
GPoint verticallyDisplacedPoint(GPoint a, GPoint b, int order);


int main() {
	GWindow gw(width, height);
    GButton *button = new GButton("Draw");
    gw.addToRegion(button, "South");
    
    GChooser *timeChooser = new GChooser();
    timeChooser->addItem("Dusk");
    timeChooser->addItem("Day");
    timeChooser->addItem("Night");
    gw.addToRegion(timeChooser, "South");
    
    GCheckBox *fg = new GCheckBox("Foreground Terrain");
    gw.addToRegion(fg, "West");
    
    GCheckBox *sun = new GCheckBox("Sun/Moon");
    gw.addToRegion(sun, "West");
    
    GCheckBox *bg = new GCheckBox("Background Mountain");
    gw.addToRegion(bg, "West");
    
    
    while(true){
        GEvent e = waitForEvent(ACTION_EVENT | CLICK_EVENT);
        if(e.getEventClass() == ACTION_EVENT) break;
    }
    string selectedTime = timeChooser->getSelectedItem();
    if(selectedTime=="Day"){
        day=true;
        dusk=false;
    }
    if(selectedTime=="Night"){
        night=true;
        dusk=false;
    }

    drawSky(gw);
    if(sun->isSelected()) drawSun(gw,MAX_SUN_RADIUS,(dusk?16723967:16777190));
    if(bg->isSelected()) drawBackGroundMountain(gw);
    if(fg->isSelected()) drawTerrain(gw);
	return 0;
}

void drawSky(GWindow gw){
    int iMax = pow(2,SKY_ORDER);
    Grid<int> squareDiamond(iMax+1,iMax+1);
    squareDiamond[0][0]=0;
    squareDiamond[0][iMax]=255/2;
    squareDiamond[iMax][0]=255/2;
    squareDiamond[iMax][iMax]=0;
    int xMin = 0;
    int xMax = iMax;
    int yMin = 0;
    int yMax = iMax;
    int randMax = 255/2;
    sdAlgo(squareDiamond, SKY_ORDER, xMin, xMax, yMin, yMax, randMax);
    skyColoring(squareDiamond,gw);
}

void skyColoring(Grid<int> & colors, GWindow gw){
    double xStep = (double)width/colors.numCols();
    double yStep = (double)height/colors.numRows();
    for(int i=0; i<colors.numCols(); i++){
        for(int j=0; j<colors.numRows(); j++){
            int color = colors[i][j]*65536+(dusk?45535:65535);
            if(night){
                color = colors[i][j]*.5;
            }
            GRect *rect = new GRect(xStep*i, yStep*j, xStep, yStep);
            rect->setFilled(true);
            rect->setColor(color);
            gw.add(rect);
            if(night){
                if(rand()%25==1){
                    GRect *star = new GRect(xStep*i, yStep*j, xStep/10, yStep/10);
                    star->setFilled(true);
                    star->setColor(16777215);
                    gw.add(star);
                }
            }
        }
    }
}

int drawSun(GWindow gw, int radius, int color){
    if(radius<=0){
        return 0;
    }
    GOval *oval;
    if(dusk){
        oval = new GOval(width/4-radius,height*.45-radius,radius*2,radius*2);
    }
    else{
        oval = new GOval(2.0*width/3.0-radius/2,height/3.0-radius/2,radius,radius);
    }
    oval->setFilled(true);
    oval->setFillColor(color);
    oval->setColor(color);
    gw.add(oval);
    radius--;
    color -=2;
    return drawSun(gw, radius, color);
}

void drawTerrain(GWindow gw){
    GPoint leftTri[3];
    leftTri[0] = GPoint(-50,height*.66);
    leftTri[1] = GPoint(-50,height+50);
    leftTri[2] = GPoint(width+50,height+50);
    foldedTriangle(TERRAIN_ORDER, leftTri, gw);
    closePoints.clear();
    GPoint rightTri[3];
    rightTri[0] = GPoint(width+50,height*.66);
    rightTri[1] = GPoint(width+50,height+50);
    rightTri[2] = GPoint(-50,height+50);
    foldedTriangle(TERRAIN_ORDER, rightTri, gw);
}

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

void drawTriangle(GPoint triangle[], GWindow gw){
    GPolygon *tri = new GPolygon();
    for(int i=0; i<3; i++){
        tri->addVertex(triangle[i].getX(), triangle[i].getY());
    }
    tri->setFilled(true);
    tri->setFillColor(5597969+rand()%100);
    gw.add(tri);
}

GPoint randomClosePoint(GPoint a, GPoint b, int order){
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


void drawBackGroundMountain(GWindow gw){
    GPolygon *mountain = mountainPolygon();
    mountain->setFilled(true);
    mountain->setColor(2640896);
    if(!day) mountain->setColor(4605769);
    gw.add(mountain);
}



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

GPoint verticallyDisplacedPoint(GPoint a, GPoint b, int order){
    double xAve = (a.getX()+b.getX())/2;
    double yAve = (a.getY()+b.getY())/2;
    double deform = BACK_MOUNTAIN_DEFORMATION/exp(BACK_MOUNTAIN_ORDER-order);
    double y = randomReal((1-deform)*yAve, (1+deform)*yAve);
    return GPoint(xAve,y);
}

int sdAlgo(Grid<int> & grid, int order, int xMin, int xMax, int yMin, int yMax, int randMax){
    if(order==0){
        return 0;
    }
    int xAvg = (xMin+xMax)/2;
    int yAvg = (yMin+yMax)/2;
    int valAvg = grid[xMin][yMin]+grid[xMin][yMax]+grid[xMax][yMin]+grid[xMax][yMax];
    valAvg /= 4;
    grid[xAvg][yAvg] = valAvg+randomInteger(0, randMax);
    int topMiddle = (grid[xMin][yMin]+grid[xMax][yMin])/2+randomInteger(0, randMax);
    int bottomMiddle = (grid[xMin][yMax]+grid[xMax][yMax])/2+randomInteger(0, randMax);
    int leftMiddle = (grid[xMin][yMin]+grid[xMin][yMax])/2+randomInteger(0, randMax);
    int rightMiddle = (grid[xMax][yMin]+grid[xMax][yMax])/2+randomInteger(0, randMax);
    grid[xMin][yAvg] = leftMiddle;
    grid[xAvg][yMin] = topMiddle;
    grid[xMax][yAvg] = rightMiddle;
    grid[xAvg][yMax] = bottomMiddle;
    randMax /=2;
    return sdAlgo(grid, order-1, xMin, xAvg, yMin, yAvg, randMax)+sdAlgo(grid, order-1, xAvg, xMax, yMin, yAvg, randMax)+sdAlgo(grid, order-1, xMin, xAvg, yAvg, yMax, randMax)+sdAlgo(grid, order-1, xAvg, xMax, yAvg, yMax, randMax);
}