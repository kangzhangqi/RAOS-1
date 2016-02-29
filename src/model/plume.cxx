/*
 * plume model
 *
 * Author: Roice (LUO Bing)
 * Date: 2016-02-26 create this file (RAOS)
 */
#include <vector>
#include <stdio.h>
#include <stdlib.h> // drand48 and srand48 on Linux
#include <time.h> // for random seed
#include "plume.h"
#include "SimConfig.h"

/*============== Filament Model ============*/
#if defined(USE_FILAMENT_MODEL)

#define MAX_NUM_FILA 100

#if defined(WIN32) || defined(__EMX__)
#  define drand48() (((float) rand())/((float) RAND_MAX))
#  define srand48(x) (srand((x)))
#elif defined __APPLE__
#  define drand48() (((float) rand())/((float) RAND_MAX))
#  define srand48(x) (srand((x)))
#endif

class FilaModel
{
    public:
        std::vector<FilaState_t> state; // state of fila, a list

        void init(void) // fila initialization
        {
            // init delta time
            dt = 0.1; // sec
            // init source pos params
            SimConfig_t *config = SimConfig_get_configs();
            source_pos[0] = config->source.x;
            source_pos[1] = config->source.y;
            source_pos[2] = config->source.z;
            /* init fila state */
            state.reserve(MAX_NUM_FILA); // make room for MAX_NUM_FILA fila
            /* release first odor pack at source */
            fila_release();
            /* clear number of fila need to release */
            fila_num_need_release = 0.0;
            /* seed the random generator */
            srand48(time(NULL));
        }
        void update(void) // update fila
        {
            float wind_x, wind_y, wind_z;
            float vm_x, vm_y, vm_z;
        /* Step 1: update positions of fila */
            for (int i = 0; i < state.size(); i++) // for each fila
            {
                // calculate wind
                wind_x = 1;
                wind_y = 0;
                wind_z = 0;
                // calculate centerline relative dispersion
                vm_x = drand48() - 0.5;
                vm_y = drand48() - 0.5;
                vm_z = drand48() - 0.5;
                // calculate pos increment
                state.at(i).x += (wind_x+vm_x) * dt;
                state.at(i).y += (wind_y+vm_y) * dt;
                state.at(i).z += (wind_z+vm_z) * dt;
            }
        /* Step 2: update sizes of fila */
            for (int i = 0; i < state.size(); i++) // for each fila
            {
                state.at(i).r += 0.01;
            }
        /* Step 3: fila maintainance */
            fila_release();
            if (state.size() > MAX_NUM_FILA)
                state.erase(state.begin());                
        }
    private: 
        float dt; // delta time
        float source_pos[3]; // source pos
        float fila_num_need_release; // "buffer" fila
        void fila_release(void) // release a filament (puff)
        {
            FilaState_t new_fila;
            new_fila.x = source_pos[0];
            new_fila.y = source_pos[1];
            new_fila.z = source_pos[2];
            new_fila.r = 0.05;
            state.push_back(new_fila);
        }
};
#endif

/*==============   Plume Model  ============*/
#if defined(USE_FILAMENT_MODEL)
FilaModel *fila;
#endif

void plume_init(void)
{
#if defined(USE_FILAMENT_MODEL)
    fila = new FilaModel();
    fila->init();
#endif
}

void plume_update(void)
{
#if defined(USE_FILAMENT_MODEL)
    fila->update();
#endif
}

#if defined(USE_FILAMENT_MODEL)
std::vector<FilaState_t>* plume_get_fila_state(void)
{
    return &fila->state;
}
#endif

/* End of plume.cxx */
