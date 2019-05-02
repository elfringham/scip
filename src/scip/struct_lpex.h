/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2018 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not visit scip.zib.de.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   struct_lp.h
 * @ingroup INTERNALAPI
 * @brief  data structures for LP management
 * @author Tobias Achterberg
 *
 *  In SCIP, the LP is defined as follows:
 *
 *     min       obj * x
 *        lhs <=   A * x + const <= rhs
 *        lb  <=       x         <= ub
 *
 *  The row activities are defined as activity = A * x + const and must
 *  therefore be in the range of [lhs,rhs].
 *
 *  Mathematically, each range constraint would account for two dual
 *  variables, one for each inequality. Since in an optimal solution (at
 *  least) one of them may be chosen to be zero, we may define one dual
 *  multiplier for each row as the difference of those two.
 *
 *  Let y be the vector of dual multipliers for the rows, then the reduced
 *  costs are defined as
 *
 *     redcost = obj - A^T * y.
 *
 *  In an optimal solution, y must be
 *
 *     - nonnegative, if the corresponding row activity is not tight at its rhs
 *     - nonpositive, if the corresponding row activity is not tight at its lhs
 *     - zero, if the corresponding row activity is not at any of its sides
 *
 *  and the reduced costs must be
 *
 *     - nonnegative, if the corresponding variable is not tight at its ub
 *     - nonpositive, if the corresponding variable is not tight at its lb
 *     - zero, if the corresponding variable is not at any of its bounds.
 *
 *  The main datastructures for storing an LP are the rows and the columns.
 *  A row can live on its own (if it was created by a separator), or as SCIP_LP
 *  relaxation of a constraint. Thus, it has a nuses-counter, and is
 *  deleted, if not needed any more.
 *  A column cannot live on its own. It is always connected to a problem
 *  variable. Because pricing is always problem specific, it cannot create
 *  LP columns without introducing new variables. Thus, each column is
 *  connected to exactly one variable, and is deleted, if the variable
 *  is deleted.
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_STRUCT_LPEX_H__
#define __SCIP_STRUCT_LPEX_H__


#include "scip/def.h"
#include "scip/scip_exact.h"
#include "scip/type_lp.h"
#include "scip/type_lpex.h"
#include "scip/type_sol.h"
#include "scip/type_var.h"
#include "scip/type_event.h"
#include "lpi/type_lpi.h"
#include "lpi/type_lpiex.h"
#include "scip/rational.h"

#ifdef __cplusplus
extern "C" {
#endif

/** collected values of a column which depend on the LP solution
 *  We store these values in each column to recover the LP solution at start of diving or probing mode, say, without
 *  having to resolve the LP.  Note that we do not store the farkascoef value since we do expect a node with infeasible
 *  LP to be pruned anyway.
 */
struct SCIP_ColSolValsEx
{
   SCIP_Rational*        primsol;            /**< primal solution value in LP, is 0 if col is not in LP */
   SCIP_Rational*        redcost;            /**< reduced cost value in LP, or SCIP_INVALID if not yet calculated */
   unsigned int          basisstatus:2;      /**< basis status of column in last LP solution, invalid for non-LP columns */
};

/** collected values of a row which depend on the LP solution
 *  We store these values in each row to recover the LP solution at start of diving or probing mode, say, without having
 *  to resolve the LP.  We do not store the dualfarkas value since we expect a node with infeasible LP to be pruned
 *  anyway. In this unlikely case, we have to resolve the LP.
 */
struct SCIP_RowSolValsEx
{
   SCIP_Rational*        dualsol;            /**< dual solution value in LP, is 0 if row is not in LP */
   SCIP_Rational*        activity;           /**< row activity value in LP, or SCIP_INVALID if not yet calculated */
   unsigned int          basisstatus:2;      /**< basis status of row in last LP solution, invalid for non-LP rows */
};

/** collected values of the LP data which depend on the LP solution
 *  We store these values to recover the LP solution at start of diving or probing mode, say, without having to resolve
 *  the LP.
 */
struct SCIP_LpSolValsEx
{
   SCIP_LPSOLVALS*       solvalsreal;
   SCIP_Rational*        lpobjval;           /**< objective value of LP without loose variables, or SCIP_INVALID */
};

/** LP column;
 *  The row vector of the LP column is partitioned into two parts: The first col->nlprows rows in the rows array
 *  are the ones that belong to the current LP (col->rows[j]->lppos >= 0) and that are linked to the column
 *  (col->linkpos[j] >= 0). The remaining col->len - col->nlprows rows in the rows array are the ones that
 *  don't belong to the current LP (col->rows[j]->lppos == -1) or that are not linked to the column
 *  (col->linkpos[j] == -1).
 */
struct SCIP_ColEx
{
   SCIP_COL*             fpcol;
   SCIP_ROWEX**          rows;
   SCIP_Rational*        obj;                /**< current objective value of column in LP (might be changed in diving or probing) */
   SCIP_Rational*        lb;                 /**< current lower bound of column in LP */
   SCIP_Rational*        ub;                 /**< current upper bound of column in LP */
   SCIP_Rational*        unchangedobj;       /**< unchanged objective value of column (ignoring diving or probing changes) */
   SCIP_Rational*        lazylb;             /**< lazy lower bound of the column; if the current lower bound is not greater than 
                                              *   the lazy lower bound, then the lower bound has not to be added to the LP */
   SCIP_Rational*        lazyub;             /**< lazy upper bound of the column; if the current upper bound is not smaller than 
                                              *   the lazy upper bound, then the upper bound has not to be added to the LP */
   SCIP_Rational*        flushedobj;         /**< objective value of column already flushed to the LP solver */
   SCIP_Rational*        flushedlb;          /**< lower bound of column already flushed to the LP solver */
   SCIP_Rational*        flushedub;          /**< upper bound of column already flushed to the LP solver */
   SCIP_Rational*        primsol;            /**< primal solution value in LP, is 0 if col is not in LP */
   SCIP_Rational*        redcost;            /**< reduced cost value in LP, or SCIP_INVALID if not yet calculated */
   SCIP_Rational*        farkascoef;         /**< coefficient in dual Farkas infeasibility proof (== dualfarkas^T A_c) */
   SCIP_Rational*        minprimsol;         /**< minimal LP solution value, this column ever assumed */
   SCIP_Rational*        maxprimsol;         /**< maximal LP solution value, this column ever assumed */
   SCIP_Rational*        sbdown;             /**< strong branching information for downwards branching */
   SCIP_Rational*        sbup;               /**< strong branching information for upwards branching */
   SCIP_Rational*        sbsolval;           /**< LP solution value of column at last strong branching call */
   SCIP_Rational*        sblpobjval;         /**< LP objective value at last strong branching call on the column */
   SCIP_VAR*             var;                /**< variable, this column represents; there cannot be a column without variable */
   SCIP_Rational**       vals;               /**< coefficients of column entries */
   SCIP_Longint          validredcostlp;     /**< LP number for which reduced cost value is valid */
   SCIP_Longint          validfarkaslp;      /**< LP number for which Farkas coefficient is valid */
   int*                  linkpos;            /**< position of col in col vector of the row, or -1 if not yet linked */
   int                   index;              /**< consecutively numbered column identifier */
   int                   size;               /**< size of the row- and val-arrays */
   int                   len;                /**< number of nonzeros in column */
   int                   nlprows;            /**< number of linked rows in column, that belong to the current LP */
   int                   nunlinked;          /**< number of column entries, where the rows don't know about the column */
   int                   lppos;              /**< column position number in current LP, or -1 if not in current LP */
   int                   lpipos;             /**< column position number in LP solver, or -1 if not in LP solver */
   unsigned int          basisstatus:2;      /**< basis status of column in last LP solution, invalid for non-LP columns */
   unsigned int          lprowssorted:1;     /**< are the linked LP rows in the rows array sorted by non-decreasing index? */
   unsigned int          nonlprowssorted:1;  /**< are the non-LP/not linked rows sorted by non-decreasing index? */
   unsigned int          objchanged:1;       /**< has objective value changed, and has data of LP solver to be updated? */
   unsigned int          lbchanged:1;        /**< has lower bound changed, and has data of LP solver to be updated? */
   unsigned int          ubchanged:1;        /**< has upper bound changed, and has data of LP solver to be updated? */
   unsigned int          coefchanged:1;      /**< has the coefficient vector changed, and has LP solver to be updated? */
   unsigned int          integral:1;         /**< is associated variable of integral type? */
   unsigned int          removable:1;        /**< is column removable from the LP (due to aging or cleanup)? */
   unsigned int          sbdownvalid:1;      /**< stores whether the stored strong branching down value is a valid dual bound;
                                              *   otherwise, it can only be used as an estimate value */
   unsigned int          sbupvalid:1;        /**< stores whether the stored strong branching up value is a valid dual bound;
                                              *   otherwise, it can only be used as an estimate value */
};

/** LP row
 *  The column vector of the LP row is partitioned into two parts: The first row->nlpcols columns in the cols array
 *  are the ones that belong to the current LP (row->cols[j]->lppos >= 0) and that are linked to the row
 *  (row->linkpos[j] >= 0). The remaining row->len - row->nlpcols columns in the cols array are the ones that
 *  don't belong to the current LP (row->cols[j]->lppos == -1) or that are not linked to the row
 *  (row->linkpos[j] == -1).
 */
struct SCIP_RowEx
{
   SCIP_ROW*             fprow;              /**< pointer to the corresponding row in the fp lp */
   SCIP_Rational*        constant;           /**< constant shift c in row lhs <= ax + c <= rhs */
   SCIP_Rational*        lhs;                /**< left hand side of row */
   SCIP_Rational*        rhs;                /**< right hand side of row */
   SCIP_Rational*        flushedlhs;         /**< left hand side minus constant of row already flushed to the LP solver */
   SCIP_Rational*        flushedrhs;         /**< right hand side minus constant of row already flushed to the LP solver */
   SCIP_Rational*        objprod;            /**< scalar product of row vector with objective function */
   SCIP_Rational*        maxval;             /**< maximal absolute value of row vector, only valid if nummaxval > 0 */
   SCIP_Rational*        minval;             /**< minimal absolute non-zero value of row vector, only valid if numminval > 0 */
   SCIP_Rational*        dualsol;            /**< dual solution value in LP, is 0 if row is not in LP */
   SCIP_Rational*        activity;           /**< row activity value in LP, or SCIP_INVALID if not yet calculated */
   SCIP_Rational*        dualfarkas;         /**< multiplier value in dual Farkas infeasibility proof */
   SCIP_Rational*        pseudoactivity;     /**< row activity value in pseudo solution, or SCIP_INVALID if not yet calculated */
   SCIP_Rational*        minactivity;        /**< minimal activity value w.r.t. the column's bounds, or SCIP_INVALID */
   SCIP_Rational*        maxactivity;        /**< maximal activity value w.r.t. the column's bounds, or SCIP_INVALID */
   SCIP_Rational**       vals;               /**< coefficients of row entries */
   void*                 origin;             /**< pointer to constraint handler or separator who created the row (NULL if unkown) */
   char*                 name;               /**< name of the row */
   SCIP_COLEX**          cols;               /**< columns of row entries, that may have a nonzero primal solution value */
   int*                  cols_index;         /**< copy of cols[i]->index for avoiding expensive dereferencing */
   int*                  linkpos;            /**< position of row in row vector of the column, or -1 if not yet linked */
   SCIP_EVENTFILTER*     eventfilter;        /**< event filter for events concerning this row */
   SCIP_Longint          validactivitylp;    /**< LP number for which activity value is valid */
   int                   index;              /**< consecutively numbered row identifier */
   int                   size;               /**< size of the col- and val-arrays */
   int                   len;                /**< number of nonzeros in row */
   int                   nlpcols;            /**< number of linked columns in row, that belong to the current LP */
   int                   nunlinked;          /**< number of row entries, where the columns don't know about the row */
   int                   nuses;              /**< number of times, this row is referenced */
   int                   lppos;              /**< row position number in current LP, or -1 if not in current LP */
   int                   lpipos;             /**< row position number in LP solver, or -1 if not in LP solver */
   int                   lpdepth;            /**< depth level at which row entered the LP, or -1 if not in current LP */
   int                   minidx;             /**< minimal column index of row entries */
   int                   maxidx;             /**< maximal column index of row entries */
   int                   numintcols;         /**< number of integral columns */
   int                   nummaxval;          /**< number of coefs with absolute value equal to maxval, zero if maxval invalid */
   int                   numminval;          /**< number of coefs with absolute value equal to minval, zero if minval invalid */
   int                   age;                /**< number of successive times this row was in LP and was not sharp in solution */
   int                   rank;               /**< rank of the row (upper bound, to be precise) */
   unsigned int          basisstatus:2;      /**< basis status of row in last LP solution, invalid for non-LP rows */
   unsigned int          lpcolssorted:1;     /**< are the linked LP columns in the cols array sorted by non-decreasing index? */
   unsigned int          nonlpcolssorted:1;  /**< are the non-LP/not linked columns sorted by non-decreasing index? */
   unsigned int          delaysort:1;        /**< should the row sorting be delayed and done in a lazy fashion? */
   unsigned int          validminmaxidx:1;   /**< are minimal and maximal column index valid? */
   unsigned int          lhschanged:1;       /**< was left hand side or constant changed, and has LP solver to be updated? */
   unsigned int          rhschanged:1;       /**< was right hand side or constant changed, and has LP solver to be updated? */
   unsigned int          coefchanged:1;      /**< was the coefficient vector changed, and has LP solver to be updated? */
   unsigned int          integral:1;         /**< is activity (without constant) of row always integral in feasible solution? */
   unsigned int          local:1;            /**< is row only valid locally? */
   unsigned int          modifiable:1;       /**< is row modifiable during node processing (subject to column generation)? */
   unsigned int          removable:1;        /**< is row removable from the LP (due to aging or cleanup)? */
   unsigned int          inglobalcutpool:1;  /**< is row contained in the global cut pool? */
   unsigned int          normunreliable:1;   /**< is the objective product of the row unreliable? */
   unsigned int          nlocks:15;          /**< number of sealed locks of an unmodifiable row */
   unsigned int          origintype:3;       /**< origin of row (0: unkown, 1: constraint handler, 2: constraint, 3: separator, 4: reoptimization) */
};

/** current LP data */
struct SCIP_LpEx
{
   SCIP_LP*              fplp;               /**< pointer to the fp lp */
   SCIP_HASHTABLE*       exrowhash;          /**< hashes fprows as keys onto exact rows */
   SCIP_HASHTABLE*       excolhash;          /**< hashes fprows as keys onto exact rows */
   SCIP_Rational*        lpobjval;           /**< objective value of LP without loose variables, or SCIP_INVALID */
   SCIP_Rational*        looseobjval;        /**< current solution value of all loose variables set to their best bounds,
                                              *   ignoring variables, with infinite best bound */
   SCIP_Rational*        rellooseobjval;     /**< last reliable solution value of all loose variables set to their best bounds,
                                              *   ignoring variables, with infinite best bound */
   SCIP_Rational*        glbpseudoobjval;    /**< global pseudo solution value with all variables set to their best global bounds,
                                              *   ignoring variables, with infinite best bound */
   SCIP_Rational*        relglbpseudoobjval; /**< last reliable global pseudo solution value */
   SCIP_Rational*        pseudoobjval;       /**< current pseudo solution value with all variables set to their best bounds,
                                              *   ignoring variables, with infinite best bound */
   SCIP_Rational*        relpseudoobjval;    /**< last reliable pseudo solution value */
   SCIP_Rational*        rootlpobjval;       /**< objective value of root LP without loose variables, or SCIP_INVALID */
   SCIP_Rational*        rootlooseobjval;    /**< objective value of loose variables in root node, or SCIP_INVALID */
   SCIP_Rational*        cutoffbound;        /**< upper objective limit of LP (copy of primal->cutoffbound) */
   SCIP_Rational*        lpiobjlim;          /**< current objective limit in LPI */
   SCIP_Rational*        objsqrnorm;         /**< squared Euclidean norm of objective function vector of problem variables */
   SCIP_Rational*        objsumnorm;         /**< sum norm of objective function vector of problem variables */
   SCIP_LPIEX*           lpiex;              /**< exact LP solver interface */
   SCIP_COLEX**          lpicols;            /**< array with columns currently stored in the LP solver */
   SCIP_ROWEX**          lpirows;            /**< array with rows currently stored in the LP solver */
   SCIP_COLEX**          chgcols;            /**< array of changed columns not yet applied to the LP solver */
   SCIP_ROWEX**          chgrows;            /**< array of changed rows not yet applied to the LP solver */
   SCIP_COLEX**          cols;               /**< array with current LP columns in correct order */
   SCIP_COLEX**          lazycols;           /**< array with current LP lazy columns */
   SCIP_ROWEX**          rows;               /**< array with current LP rows in correct order */
   SCIP_Rational**       soldirection;       /**< normalized vector in direction of primal solution from current LP solution */
   SCIP_LPSOLVALSEX*     storedsolvals;      /**< collected values of the LP data which depend on the LP solution */
   SCIP_SOL*             validsoldirsol;     /**< primal solution for which the currently stored solution direction vector is valid */
   int                   lpicolssize;        /**< available slots in lpicols vector */
   int                   nlpicols;           /**< number of columns in the LP solver */
   int                   lpifirstchgcol;     /**< first column of the LP which differs from the column in the LP solver */
   int                   lpirowssize;        /**< available slots in lpirows vector */
   int                   nlpirows;           /**< number of rows in the LP solver */
   int                   lpifirstchgrow;     /**< first row of the LP which differs from the row in the LP solver */
   int                   chgcolssize;        /**< available slots in chgcols vector */
   int                   nchgcols;           /**< current number of chgcols (number of used slots in chgcols vector) */
   int                   chgrowssize;        /**< available slots in chgrows vector */
   int                   nchgrows;           /**< current number of chgrows (number of used slots in chgrows vector) */
   int                   colssize;           /**< available slots in cols vector */
   int                   soldirectionsize;   /**< available slots in soldirection vector */
   int                   ncols;              /**< current number of LP columns (number of used slots in cols vector) */
   int                   nremovablecols;     /**< number of removable columns in the LP */
   int                   firstnewcol;        /**< first column added at the current node */
   int                   rowssize;           /**< available slots in rows vector */
   int                   nrows;              /**< current number of LP rows (number of used slots in rows vector) */
   int                   nremovablerows;     /**< number of removable rows in the LP */
   int                   firstnewrow;        /**< first row added at the current node */
   int                   looseobjvalinf;     /**< number of loose variables with infinite best bound in current solution */
   int                   nloosevars;         /**< number of loose variables in LP */
   int                   glbpseudoobjvalinf; /**< number of variables with infinite best bound in global pseudo solution */
   int                   pseudoobjvalinf;    /**< number of variables with infinite best bound in current pseudo solution */
   int                   lpiitlim;           /**< current iteration limit setting in LPI */
   int                   lpitiming;          /**< current timing type in LPI */
   int                   lpirandomseed;      /**< current initial random seed in LPI */
   int                   lpiscaling;         /**< current SCALING setting in LPI */
   int                   lpirefactorinterval;/**< current refactorization interval */
   int                   interleavedbfreq;   /**< frequency at which dual bounding strategy is interleaved (-1: never, 0: if prommising, x: xth node) */
   int                   ninfiniteboundcols; /**< number of columns with infinity upper or lower bounds (important for safe bounding) */
   SCIP_PRICING          lpipricing;         /**< current pricing setting in LPI */
   SCIP_LPSOLSTAT        lpsolstat;          /**< solution status of last LP solution */
   SCIP_LPALGO           lastlpalgo;         /**< algorithm used for last LP solve */
   SCIP_Bool             lpisolutionpolishing;/**< LP solution polishing method (0: disabled, 1: enabled) */
   SCIP_Bool             flushdeletedcols;   /**< have LPI-columns been deleted in the last lpFlush() call? */
   SCIP_Bool             flushaddedcols;     /**< have LPI-columns been added in the last lpFlush() call? */
   SCIP_Bool             flushdeletedrows;   /**< have LPI-rows been deleted in the last lpFlush() call? */
   SCIP_Bool             flushaddedrows;     /**< have LPI-rows been added in the last lpFlush() call? */
   SCIP_Bool             updateintegrality;  /**< does integrality information need to be updated? */
   SCIP_Bool             flushed;            /**< are all cached changes applied to the LP solver? */
   SCIP_Bool             solved;             /**< is current LP solved? */
   SCIP_Bool             primalfeasible;     /**< is current LP solution (rather LPI state) primal feasible? */
   SCIP_Bool             primalchecked;      /**< was current LP solution checked for primal feasibility?? */
   SCIP_Bool             dualfeasible;       /**< is current LP solution (rather LPI state) dual feasible? */
   SCIP_Bool             dualchecked;        /**< was current LP solution checked for primal feasibility?? */
   SCIP_Bool             solisbasic;         /**< is current LP solution a basic solution? */
   SCIP_Bool             rootlpisrelax;      /**< is root LP a relaxation of the problem and its solution value a valid global lower bound? */
   SCIP_Bool             isrelax;            /**< is the current LP a relaxation of the problem for which it has been solved and its 
                                              *   solution value a valid local lower bound? */
   SCIP_Bool             installing;         /**< whether the solution process is in stalling */
   SCIP_Bool             resolvelperror;     /**< an error occured during resolving the LP after diving or probing */
   SCIP_Bool             adjustlpval;        /**< does an infinite LP objective value has been adjusted so far? */
   SCIP_Bool             lpihasscaling;      /**< does the LPI support the SCALING parameter? */
   SCIP_Bool             lpihaspresolving;   /**< does the LPI support the PRESOLVING parameter? */
   SCIP_Bool             projshiftpossible;  /**< can a safe bound be computed with project-and-shift? */
};

#ifdef __cplusplus
}
#endif

#endif