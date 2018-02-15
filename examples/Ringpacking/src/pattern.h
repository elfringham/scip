/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2015 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   pattern.h
 * @brief  pattern data for ringpacking problem
 * @author Benjamin Mueller
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_PATTERN__
#define __SCIP_PATTERN__

#include "scip/scip.h"

/*
 * data structures
 */

enum SCIP_Packable
{
   SCIP_PACKABLE_NO      = 0,
   SCIP_PACKABLE_YES     = 1,
   SCIP_PACKABLE_UNKNOWN = 2
};
typedef enum SCIP_Packable SCIP_PACKABLE;

enum SCIP_Patterntype
{
   SCIP_PATTERNTYPE_CIRCULAR    = 0,           /**< circular pattern */
   SCIP_PATTERNTYPE_RECTANGULAR = 1            /**< rectangular pattern */
};
typedef enum SCIP_Patterntype SCIP_PATTERNTYPE;

struct SCIP_Pattern
{
   SCIP_PATTERNTYPE      patterntype;        /**< pattern type */
   SCIP_PACKABLE         packable;           /**< packable status */
   int*                  nelems;             /**< array to store the elements of each type */
   int                   ntypes;             /**< total number of types */
   int                   nlocks;             /**< number of locks */
   int                   type;               /**< type of the boundary circle */
};
typedef struct SCIP_Pattern SCIP_PATTERN;

/** creates an empty circular pattern */
extern
SCIP_RETCODE SCIPpatternCreateCircular(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_PATTERN**        pattern,            /**< pointer to store pattern */
   int                   ntypes,             /**< number of different circle types */
   int                   type                /**< circle type (not needed for rectangular patterns) */
   );

/** creates an empty circular pattern */
extern
SCIP_RETCODE SCIPpatternCreateRectangular(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_PATTERN**        pattern,            /**< pointer to store pattern */
   int                   ntypes              /**< number of different circle types */
   );

/** captures a pattern */
extern
void SCIPpatternCapture(
   SCIP_PATTERN*         pattern             /**< pattern */
   );

/* frees a pattern */
extern
void SCIPpatternRelease(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_PATTERN**        pattern             /**< pointer to free pattern */
   );

/** adds an element of a given type to a pattern */
extern
void SCIPpatternAddElement(
   SCIP_PATTERN*         pattern,            /**< pattern */
   int                   type                /**< element of a given type */
   );

/** removes an element of a given type from a pattern */
extern
void SCIPpatternRemoveElement(
   SCIP_PATTERN*         pattern,            /**< pattern */
   int                   type                /**< element of a given type */
   );

/** returns the number of elements of a given type in the pattern */
extern
int SCIPpatternGetNElemens(
   SCIP_PATTERN*         pattern,            /**< pattern */
   int                   type                /**< element of a given type */
   );

/** returns the type of a pattern */
extern
SCIP_PATTERNTYPE SCIPpatternGetPatternType(
   SCIP_PATTERN*         pattern             /**< pattern */
   );

/** returns the type of the boundary circle
 *
 * @note this function can only be called for circular patterns
 */
extern
int SCIPpatternGetType(
   SCIP_PATTERN*         pattern             /**< pattern */
   );

/** returns the packable status of a pattern */
extern
SCIP_PACKABLE SCIPpatternGetPackableStatus(
   SCIP_PATTERN*         pattern             /**< pattern */
   );

/** sets the packable status of a pattern */
extern
void SCIPpatternSetPackableStatus(
   SCIP_PATTERN*         pattern,            /**< pattern */
   SCIP_PACKABLE         packable            /**< packable status */
   );

#endif /* __SCIP_PATTERN__ */