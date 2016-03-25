/*
 * single-rotor wake model running on GPU
 *      Free vortex method
 *
 * Author: Roice (LUO Bing)
 * Date: 2016-03-02 create this file (RAOS)
 *       2016-03-08 CPU too slow, modified to GPU
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h> // drand48 and srand48 on Linux
#include <string.h> // memset
#include "cblas.h" // CBLAS for vector arithmetic
#include "model/wake_rotor.h"

#define PI 3.14159265358979323846

void RotorWake::init(void) // rotor wake initialization
{// by far config and rotor_state should be artificially configured

    // init vortex markers state
    max_markers = int(360.0*config.rounds/config.dpsi)+1; // calculate max lagrangian markers of a vortex filament, 10.0 degree is an arbitrary number
    wake_state = (std::vector<VortexMarker_t>**)malloc(rotor_state.frame.n_blades*sizeof(std::vector<VortexMarker_t>*));
    for (int i = 0; i < rotor_state.frame.n_blades; i++) {
        wake_state[i] = new std::vector<VortexMarker_t>;
        wake_state[i]->reserve(max_markers+10); // 10 is an arbitrary number, can be num>=1
    }

    // initial wake geometry, using Landgrebe's prescribed wake model
    // < Principles of helicopter >, JG Leishman, page 604-606
    init_wake_geometry();

    // release first group of markers (one for each blade) at the tips
    //marker_release();
}

void RotorWake::maintain(void) 
{/* marker maintainance */
    
    // release new markers
    marker_release();
    
    // remove old markers
    if (wake_state[0]->size() > max_markers) {
        for (int i = 0; i < rotor_state.frame.n_blades; i++)
            wake_state[i]->erase(wake_state[i]->begin());
    }
}

void RotorWake::marker_release(void) {

    VortexMarker_t new_marker;
    float p_marker[3] = {rotor_state.frame.radius, 0, 0};

    // clear velocity
    memset(new_marker.vel, 0, sizeof(new_marker.vel));

    // calculate the tip vortex circulation: Gamma
    new_marker.Gamma = -2.0f*rotor_state.thrust/rotor_state.frame.n_blades/(rotor_state.frame.radius*rotor_state.frame.radius)/1.185f/rotor_state.Omega; // tip vortex circulation

    for (int i = 0; i < rotor_state.frame.n_blades; i++) {
        // calculate the position of this marker     
        memcpy(new_marker.pos, rotor_state.pos, sizeof(new_marker.pos));
        rotate_vector(p_marker, new_marker.pos, 360.0/rotor_state.frame.n_blades*i+rotor_state.psi+rotor_state.attitude[0], rotor_state.attitude[1], rotor_state.attitude[2]);
        // push to wake state array
        wake_state[i]->push_back(new_marker);
    }

    // update blade azimuth
    rotor_state.psi += config.dpsi;
    if (rotor_state.psi >= 360.0)
        rotor_state.psi -= 360.0;
}

void RotorWake::init_wake_geometry(void) {
    VortexMarker_t new_marker;
    float p_marker[3] = {0, 0, 0};
    float z_tip, r_tip, C_T, sigma, psi_rad;
    
    // clear velocity
    memset(new_marker.vel, 0, sizeof(new_marker.vel));
    
    // calculate the tip vortex circulation: Gamma
    new_marker.Gamma = -2.0f*rotor_state.thrust/rotor_state.frame.n_blades/(rotor_state.frame.radius*rotor_state.frame.radius)/1.185f/rotor_state.Omega; // tip vortex circulation

    // calculate thrust coefficient and sigma
    C_T = rotor_state.thrust/(1.185f*PI*pow(rotor_state.Omega,2)*pow(rotor_state.frame.radius,4));
    sigma = rotor_state.frame.n_blades*rotor_state.frame.chord/(PI*rotor_state.frame.radius);

    for (int j = max_markers-1; j >= 0; j--) {
        // calculate psi
        psi_rad = j*config.dpsi*PI/180.0f;
        // calculate r_tip
        r_tip = rotor_state.frame.radius*(0.78f+(1.0f-0.78f)*exp(-(0.145+27*C_T)*psi_rad));
        p_marker[0] = r_tip;
        // calculate z_tip
        if (psi_rad <= 2*PI/rotor_state.frame.n_blades)
            z_tip = rotor_state.frame.radius*(psi_rad*(-0.25)*(C_T/sigma)); // assume blade twist is 10 deg
        else
            z_tip = rotor_state.frame.radius*(
                    2*PI/rotor_state.frame.n_blades*(-0.25)*(C_T/sigma)
                    + (psi_rad-2*PI/rotor_state.frame.n_blades)
                    * (-1.41*sqrt(C_T/2.0))); 
        for (int i = 0; i < rotor_state.frame.n_blades; i++) {
            // calculate the position of this marker     
            memcpy(new_marker.pos, rotor_state.pos, sizeof(new_marker.pos));
            new_marker.pos[2] += z_tip*0.5;
            rotor_state.psi = -(psi_rad - floor(psi_rad/(2*PI))*(2*PI))*180.0f/PI;
            rotate_vector(p_marker, new_marker.pos, rotor_state.psi + 360.0/rotor_state.frame.n_blades*i+rotor_state.psi+rotor_state.attitude[0], rotor_state.attitude[1], rotor_state.attitude[2]);
            // push to wake state array
            wake_state[i]->push_back(new_marker);
        }
    }

    // save psi
    
}

// Constructor
RotorWake::RotorWake(void)
{
    // init configurations
    config.rounds = 20; // 20 rounds
    config.dpsi = 10; // 10 deg

    // init rotor state
    rotor_state.frame.radius = 0.1; // meter
    rotor_state.frame.chord = 0.01; // meter
    rotor_state.frame.n_blades = 2; // two-blade
    memset(rotor_state.pos, 0, sizeof(rotor_state.pos));
    memset(rotor_state.attitude, 0, sizeof(rotor_state.attitude));
    rotor_state.Omega = 25*2*PI; // rad/s
    rotor_state.psi = 0;
    rotor_state.thrust = 1.0; // 1 N, ~100 g
    
    max_markers = 1000;
}

/* End of file wake_rotor.cxx */
