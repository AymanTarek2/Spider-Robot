#ifndef MOVEMENT_STATES_H
#define MOVEMENT_STATES_H

#ifdef __cplusplus
extern "C" {
#endif

// Define the struct
typedef struct {
    int forward;
    int backward;
    int right;
    int left;
} MovementStates;

// Declare the global instance
extern MovementStates movement_states;

#ifdef __cplusplus
}
#endif

#endif // MOVEMENT_STATES_H
