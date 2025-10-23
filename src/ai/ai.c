#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "ai.h"
#include "gate.h"
#include "radix.h"
#include "utils.h"

#define DEBUG 0

#define UP 'u'
#define DOWN 'd'
#define LEFT 'l'
#define RIGHT 'r'
char directions[] = {UP, DOWN, LEFT, RIGHT};
char invertedDirections[] = {DOWN, UP, RIGHT, LEFT};
char pieceNames[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/**
 * Given a game state, work out the number of bytes required to store the state.
*/
int getPackedSize(gate_t *gate);

/**
 * Store state of puzzle in map.
*/
void packMap(gate_t *gate, unsigned char *packedMap);

/**
 * Check if the given state is in a won state.
 */
bool winning_state(gate_t gate);

gate_t* duplicate_state(gate_t* gate) {

	gate_t *duplicate = (gate_t *)malloc(sizeof(gate_t));
	assert(duplicate);
	/* Fill in */
	/*
	Hint: copy the src state as well as the dynamically allocated map and solution
	*/
	duplicate->buffer = gate->buffer;
	duplicate->map_save = gate->map_save;
	duplicate->lines = gate->lines;
	duplicate->player_x = gate->player_x;
	duplicate->player_y = gate->player_y;
	duplicate->base_path = gate->base_path;
	duplicate->num_chars_map = gate->num_chars_map;
	duplicate->num_pieces = gate->num_pieces;


	duplicate->map = malloc(sizeof(char *)* gate->lines);
	assert(duplicate->map);

	//copy map
	for (int row = 0; row < gate->lines; row++)
	{
		duplicate->map[row] = strdup(gate->map[row]);
	}

	if (gate->soln)
	{
		duplicate->soln = strdup(gate->soln);
	}
	else
	{
		duplicate->soln = NULL;
	}

	// copy piece pos
	for (int i = 0; i < MAX_PIECES; i++)
	{
		duplicate->piece_x[i] = gate->piece_x[i];
		duplicate->piece_y[i] = gate->piece_y[i];
	}

    return duplicate;
}

/**
 * Without lightweight states, the second argument may not be required.
 * Its use is up to your decision.
 */
void free_state(gate_t* stateToFree, gate_t *init_data) {
    if (!stateToFree) return;

    // Free map strings and the map array
    if (stateToFree->map) {
        for (int i = 0; i < stateToFree->lines; i++) {
            if (stateToFree->map[i]) {
                free(stateToFree->map[i]);
            }
        }
        free(stateToFree->map);
    }

    // Free solution string if it exists
    if (stateToFree->soln) {
        free(stateToFree->soln);
    }

    // Free the state structure itself
    free(stateToFree);
}

void free_initial_state(gate_t *init_data) {
	/* Frees dynamic elements of initial state data - including 
		unchanging state. */
	/* 
	Hint:
	Unchanging state:
	buffer
	map_save
	*/
		if(init_data->buffer) 
		{
			free(init_data->buffer);
		}
		if(init_data->map_save) 
		{
			for(int i = 0; i < init_data->lines; i++) 
			{
				if(init_data->map_save[i]) 
				{
					free(init_data->map_save[i]);
				}
			}
			free(init_data->map_save);
		}
}

/**
 * Find a solution by exploring all possible paths
 */
void find_solution(gate_t* init_data) {
	/* Location for packedMap. */
	int packedBytes = getPackedSize(init_data);
	unsigned char *packedMap = (unsigned char *) calloc(packedBytes, sizeof(unsigned char));
	assert(packedMap);

	bool has_won = false;
	int dequeued = 0;
	int enqueued = 0;
	int duplicatedNodes = 0;
	char *soln = "";
	double start = now();
	double elapsed;
	
	// Algorithm 1 is a width n + 1 search
	int w = init_data->num_pieces + 1;

	// Initialise queue
	queue_ptr queue = malloc(sizeof(queue_t));
	queue->head = NULL;
	queue->tail = NULL;
	queue->queuelen = 0;



	
	// Algorithm 1 - BFS
	
	// Enqueue initial state
	gate_t *init_state = duplicate_state(init_data);
	enqueue(init_state, queue);
	enqueued++;

	// Algorithm 2 - Alg 1 + RADIX

	//Set up radix tree
	// int width = init_data->num_chars_map/init_data->lines;
	// struct radixTree *radtree = getNewRadixTree(init_data->num_pieces, init_data->lines, width);
	// int atomCount = init_data->num_pieces;

	// Algorithm 3 - Alg 1 + Alg 2 + Iterated Width + Novelty

	//for (int width = 1; width < init_data->num_pieces; width++)
	//{
		// initialise radix tree based on width
		gate_t *current;
		while (queue->queuelen)
		{
			current = dequeue(queue);
			dequeued ++;

			if (winning_state(*current))
			{
				has_won = true;
				soln = strdup(current->soln);
				free_state(current, init_data);
				free_queue(queue, init_data);
				break;
			}

			// iterate over each move
			for (int p = 0; p < init_data->num_pieces; p++)
			{
				for (int m = 0; m < 4; m++)
				{
					gate_t *new_node = duplicate_state(current);
					*new_node = attempt_move(*new_node, pieceNames[p], directions[m]);

					
					// check if move is valid
					if (state_change(current, new_node))
					{
						//packMap(new_node, packedMap);
						
						// check if the move isnt already in radix tree
						//if (!checkPresent(radtree, packedMap, atomCount))
						//{
							//add to solution path
							append_sol(new_node, p, m);
							enqueue(new_node, queue);
							enqueued++;

							//add to rad tree
							//insertRadixTree(radtree, packedMap, init_data->num_pieces);						
						//}
						//else
						//{
							//duplicatedNodes++;
							//free_state(new_node, init_data);
						//}

					}
					else
					{
						free_state(new_node, init_data);
					}
				}
			}
			free_state(current, init_data);
		}

		free_queue(queue, init_data);

		if (!has_won)
		{
			printf("No solution found - puzzle is unsolvable\n");
			soln = "UNSOLVABLE";
		}

	//}
	
	/*
	 * FILL IN: Algorithm 1 - 3.
	 */
	
	/* Output statistics */
	elapsed = now() - start;
	printf("Solution path: ");
	printf("%s\n", soln);
	printf("Execution time: %lf\n", elapsed);
	printf("Expanded nodes: %d\n", dequeued);
	printf("Generated nodes: %d\n", enqueued);
	printf("Duplicated nodes: %d\n", duplicatedNodes);
	int memoryUsage = 0;
	// Algorithm 2: Memory usage, uncomment to add.
	//memoryUsage += queryRadixMemoryUsage(radtree);
	// Algorithm 3: Memory usage, uncomment to add.
	// for(int i = 0; i < w; i++) {
	//	memoryUsage += queryRadixMemoryUsage(rts[i]);
	// }
	printf("Auxiliary memory usage (bytes): %d\n", memoryUsage);
	printf("Number of pieces in the puzzle: %d\n", init_data->num_pieces);
	printf("Number of steps in solution: %ld\n", strlen(soln)/2);
	int emptySpaces = 0;
	/*
	 * FILL IN: Add empty space check for your solution.
	 */
	
	printf("Number of empty spaces: %d\n", emptySpaces);
	printf("Solved by IW(%d)\n", w);
	printf("Number of nodes expanded per second: %lf\n", (dequeued + 1) / elapsed);

	/* Free associated memory. */
	if(packedMap) {
		free(packedMap);
	}
	/* Free initial map. */
	free_initial_state(init_data);
	if (soln)
	{
	free(soln);
	}
}

/**
 * Given a game state, work out the number of bytes required to store the state.
*/
int getPackedSize(gate_t *gate) {
	int pBits = calcBits(gate->num_pieces);
    int hBits = calcBits(gate->lines);
    int wBits = calcBits(gate->num_chars_map / gate->lines);
    int atomSize = pBits + hBits + wBits;
	int bitCount = atomSize * gate->num_pieces;
	return bitCount;
}

/**
 * Store state of puzzle in map.
*/
void packMap(gate_t *gate, unsigned char *packedMap) {
	int pBits = calcBits(gate->num_pieces);
    int hBits = calcBits(gate->lines);
    int wBits = calcBits(gate->num_chars_map / gate->lines);
	int bitIdx = 0;
	for(int i = 0; i < gate->num_pieces; i++) {
		for(int j = 0; j < pBits; j++) {
			if(((i >> j) & 1) == 1) {
				bitOn( packedMap, bitIdx );
			} else {
				bitOff( packedMap, bitIdx );
			}
			bitIdx++;
		}
		for(int j = 0; j < hBits; j++) {
			if(((gate->piece_y[i] >> j) & 1) == 1) {
				bitOn( packedMap, bitIdx );
			} else {
				bitOff( packedMap, bitIdx );
			}
			bitIdx++;
		}
		for(int j = 0; j < wBits; j++) {
			if(((gate->piece_x[i] >> j) & 1) == 1) {
				bitOn( packedMap, bitIdx );
			} else {
				bitOff( packedMap, bitIdx );
			}
			bitIdx++;
		}
	}
}

/**
 * Check if the given state is in a won state.
 */
bool winning_state(gate_t gate) {
	for (int i = 0; i < gate.lines; i++) {
		for (int j = 0; gate.map_save[i][j] != '\0'; j++) {
			if (gate.map[i][j] == 'G' || (gate.map[i][j] >= 'I' && gate.map[i][j] <= 'Q')) {
				return false;
			}
		}
	}
	return true;
}

void solve(char const *path)
{
	/**
	 * Load Map
	*/
	gate_t gate = make_map(path, gate);
	
	/**
	 * Verify map is valid
	*/
	map_check(gate);

	/**
	 * Locate player x, y position
	*/
	gate = find_player(gate);

	/**
	 * Locate each piece.
	*/
	gate = find_pieces(gate);
	
	gate.base_path = path;
	gate.soln = NULL;

	find_solution(&gate);

}

void 
enqueue(gate_t* data, queue_ptr queue)
{
	// Create new node
	queuenode_ptr newNode = (queuenode_ptr)malloc(sizeof(queuenode_t));
	newNode->data = data;
	newNode->next = NULL;
	// If queue is empty
	if (queue->tail == NULL) 
	{
		queue->head = newNode;
		queue->tail = newNode;
	} 
	else 
	{
		queue->tail->next = newNode;
		queue->tail = newNode;
	}
	queue->queuelen++;
}
gate_t*
dequeue(queue_ptr queue)
{
    if (queue->head == NULL) 
    {
        // Queue is empty
		queue->tail = NULL;
        return NULL;
    }
    // Get the front node
	queuenode_ptr oldHead = queue->head;
    // Move head to next node
    gate_t *tmp = oldHead->data;

	queue->head = queue->head->next;
	// if the list is now empty
	if (queue->head == NULL) 
    {
		queue->tail = NULL;
    }

	free(oldHead);
    queue->queuelen--;
	return tmp;
}

void
free_queue(queue_ptr queue, gate_t *data)
{
	queuenode_ptr current = queue->head;
	queuenode_ptr next;
	while (current)
	{
		next = current->next;
		if (current->data)
		{
			free_state(current->data, data);
		}

		free(current);
		current = next;

	}
	

	queue->head = NULL;
	queue->tail = NULL;
	free(queue);
}

int 
state_change(gate_t * old_state, gate_t *new_state)
{
	// check if puzzle state has changed
	for (int i = 0; i < old_state->num_pieces; i++)
	{
		if (old_state->piece_x[i] != new_state->piece_x[i] || 
		    old_state->piece_y[i] != new_state->piece_y[i])
		{
			return 1; 
		}
	}
	return 0;
}

void
append_sol(gate_t *node, int piece, int move)
{
	int tmplen;
	if (!node->soln)
	{
		tmplen = 0;
	}
	else
	{
		tmplen = strlen(node->soln);
	}

	char *newsol = malloc(tmplen + 3);
	if (tmplen)
	{
		strcpy(newsol, node->soln);
	}
	newsol[tmplen] = pieceNames[piece];
	newsol[tmplen + 1] = directions[move];
	newsol[tmplen + 2] = '\0';

	if (node->soln)
	{
		free(node->soln);
	}

	node->soln = newsol;
}