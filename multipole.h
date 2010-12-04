/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore methods to interactively animate an image sequence.
*/
#ifdef hpux
#define _HPUX_SOURCE 1
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(__STDC__) || defined(sgi) || defined(_AIX)
#include <unistd.h>
#else
#include <malloc.h>
#include <memory.h>
#endif
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
/*
  Define declarations for the N-body program.
*/
#ifdef WIN32
#undef stderr
#define stderr stdout
#endif
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))
#ifndef False
#define False  0
#endif
#define CircumscribedSphereRadius  1.5  /* choose from: 3.0, 2.5, 2.0, 1.5 */
#define GravitationalConstant  1.0e-3
#define HighestDegreeHarmonic  20
#define Max(x,y)  (((x) > (y)) ? (x) : (y))
#define MaxChildren  8
#define MaxNodesInteractiveField  189  /* choose from: 140, 119, 77, 56 */
#define MaxNodesNearField  26  /* choose from: 80, 56, 32, 26 */
#define MaxTreeDepth  8  /* Log2(MaxRange) */
#define Min(x,y)  (((x) < (y)) ? (x) : (y))
#define NodesInAQueue  4681  /* 1+8+64+512+4096 */
#ifndef RAND_MAX
#define RAND_MAX  32767
#endif
#define TetrahedralArray(array,i,j,k)  \
  array[cube.tetra_three[i]-cube.tetra_two[(i)+(j)]+(k)]
#ifndef True
#define True  1
#endif
#define Warning(message,qualifier)  \
{  \
  (void) fprintf(stderr,"%s: %s",application_name,message);  \
  if (qualifier != (char *) NULL)  \
    (void) fprintf(stderr," (%s)",qualifier);  \
  (void) fprintf(stderr,".\n");  \
}

/*
  Structures.
*/
typedef struct _Particle
{
  float
    x,
    y,
    z;

  struct _Particle
    *next;
} Particle;

typedef struct _Node
{
  struct _Node
    *parent,
    *child[MaxChildren],
    *near_field[MaxNodesNearField+1],
    *interactive_field[MaxNodesInteractiveField+8];

  unsigned long
    id,
    level;

  float
    mid_x,
    mid_y,
    mid_z,
    *phi,
    *psi;

  Particle
    *particle;
} Node;

typedef struct _Nodes
{
  Node
    nodes[NodesInAQueue];

  float
    *phi,
    *psi;

  struct _Nodes
    *next;
} Nodes;

typedef struct _Cube
{
  Node
    *root;

  unsigned long
    depth;

  long
    level;

  Node
    **near_neighbor;

  unsigned long
    nodes,
    free_nodes;

  Node
    *next_node;

  Nodes
    *node_queue;

  float
    edge_length[MaxTreeDepth],
    diagonal_length[MaxTreeDepth],
    binomial[HighestDegreeHarmonic+1][HighestDegreeHarmonic+1],
    one_power[HighestDegreeHarmonic+1],
    x_power[HighestDegreeHarmonic+1],
    y_power[HighestDegreeHarmonic+1],
    z_power[HighestDegreeHarmonic+1];

  unsigned long
    tetra_two[HighestDegreeHarmonic+3],
    tetra_three[HighestDegreeHarmonic+3],
    precision,
    coefficients;

  float
    *ijk_binomial,
    *ijk_factorial,
    *phi,
    *phi_tilde,
    *psi,
    *psi_tilde;

  double
    near_potential,
    far_potential;
} Cube;
/*
  Variable declarations.
*/
char
  *application_name;
/*
  Forward declarations.
*/
static double
  MultipolePotential(Particle *,unsigned long,unsigned long,unsigned long,
    double,double),
  NaivePotential(Particle *,unsigned long);

static long
  ProcessTime(void);

static Node
  *InitializeNode(unsigned long,unsigned long,Node *,double,double,double);

static unsigned long
  Binomial(long,long),
  InNearField(register Node *,register Node *);

static void
  Classification(Particle *,unsigned long),
  ComputePhi(register Node *),
  ComputePsi(register Node *),
  CreateCube(register Node *),
  DefineInteractiveField(register Node *),
  DefineNearField(register Node *),
  Error(char *,char *),
  EvaluatePotential(Node *node),
  FindNeighbors(register Node *,register Node *),
  LocalExpansion(Node *, Node *,float *),
  MultipoleExpansion(Node *,Node *,register float *);
