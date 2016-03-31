/*
 * Quadrotor's FVM result drawing
 *          using OpenGL GLUT
 *
 * This file contains stuff to draw the wake of a quadrotor
 * using OpenGL GLUT.
 * The declarations of the classes or functions are written in 
 * file draw_wake.h, which is included by DrawScene.h
 *
 * Author: Roice (LUO Bing)
 * Date: 2016-03-07 create this file
 */
#include <FL/gl.h>
#include <vector>
#include "model/robot.h"
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

/* draw a vortex filament */
static void draw_vortex_filament(const std::vector<VortexMarker_t>* markers)
{
    int n_m = markers->size();

    // draw
    glDisable(GL_LIGHTING);

    glColor3f(0.0, 1.0, 0.0); /* green */
    glBegin(GL_LINES);
    for (int i = 0; i < n_m-1; i++)
    {// draw each segment as line segment    
  	    glVertex3f(markers->at(i).pos[0],
                markers->at(i).pos[2], 
                -markers->at(i).pos[1]);
        glVertex3f(markers->at(i+1).pos[0],
                markers->at(i+1).pos[2], 
                -markers->at(i+1).pos[1]);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

/* draw a vortex ring */
static void draw_vortex_ring(const FarWakeState_t* ring)
{
    float x, y, theta;
    int n = 10; // 10 segments

    // draw
    glDisable(GL_LIGHTING);

    glColor3f(0.0, 0.0, 1.0); /* blue */
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < n; i++)
    {
        theta = 2.0f*PI*i/n;
        x = cosf(theta)*ring->radius;
        y = sinf(theta)*ring->radius;
        glVertex3f(ring->center[0]+x, ring->center[2], ring->center[1]+y);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

/* draw the wake generated by quad rotor */
void draw_wake(std::vector<Robot*>* robots)
{
    /* draw FVM vortex filaments */
    for (int i = 0; i < robots->size(); i++)
        for (int j = 0; j < robots->at(i)->wakes.size(); j++)
            for (int k = 0; k < robots->at(i)->wakes.at(j)->rotor_state.frame.n_blades; k++)
                draw_vortex_filament(robots->at(i)->wakes.at(j)->wake_state[k]);
    /* draw far wake boundary condition */
    for (int i = 0; i < robots->size(); i++)
        for (int j = 0; j < robots->at(i)->wakes.size(); j++)
            draw_vortex_ring(&(robots->at(i)->wakes.at(j)->far_wake));
}
