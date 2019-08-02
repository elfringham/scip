/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2019 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not visit scip.zib.de.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   scip_sol.c
 * @brief  public methods for solutions
 * @author Tobias Achterberg
 * @author Timo Berthold
 * @author Gerald Gamrath
 * @author Robert Lion Gottwald
 * @author Stefan Heinz
 * @author Gregor Hendel
 * @author Thorsten Koch
 * @author Alexander Martin
 * @author Marc Pfetsch
 * @author Michael Winkler
 * @author Kati Wolter
 *
 * @todo check all SCIP_STAGE_* switches, and include the new stages TRANSFORMED and INITSOLVE
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#else
#include <strings.h> /*lint --e{766}*/
#endif


#include "lpi/lpi.h"
#include "nlpi/exprinterpret.h"
#include "nlpi/nlpi.h"
#include "scip/benders.h"
#include "scip/benderscut.h"
#include "scip/branch.h"
#include "scip/branch_nodereopt.h"
#include "scip/clock.h"
#include "scip/compr.h"
#include "scip/concsolver.h"
#include "scip/concurrent.h"
#include "scip/conflict.h"
#include "scip/conflictstore.h"
#include "scip/cons.h"
#include "scip/cons_linear.h"
#include "scip/cutpool.h"
#include "scip/cuts.h"
#include "scip/debug.h"
#include "scip/def.h"
#include "scip/dialog.h"
#include "scip/dialog_default.h"
#include "scip/disp.h"
#include "scip/event.h"
#include "scip/heur.h"
#include "scip/heur_ofins.h"
#include "scip/heur_reoptsols.h"
#include "scip/heur_trivialnegation.h"
#include "scip/heuristics.h"
#include "scip/history.h"
#include "scip/implics.h"
#include "scip/interrupt.h"
#include "scip/lp.h"
#include "scip/lpex.h"
#include "scip/mem.h"
#include "scip/message_default.h"
#include "scip/misc.h"
#include "scip/nlp.h"
#include "scip/nodesel.h"
#include "scip/paramset.h"
#include "scip/presol.h"
#include "scip/presolve.h"
#include "scip/pricer.h"
#include "scip/pricestore.h"
#include "scip/primal.h"
#include "scip/prob.h"
#include "scip/prop.h"
#include "scip/reader.h"
#include "scip/relax.h"
#include "scip/reopt.h"
#include "scip/retcode.h"
#include "scip/scipbuildflags.h"
#include "scip/scipcoreplugins.h"
#include "scip/scipgithash.h"
#include "scip/sepa.h"
#include "scip/sepastore.h"
#include "scip/set.h"
#include "scip/sol.h"
#include "scip/solex.h"
#include "scip/solve.h"
#include "scip/stat.h"
#include "scip/syncstore.h"
#include "scip/table.h"
#include "scip/tree.h"
#include "scip/var.h"
#include "scip/varex.h"
#include "scip/visual.h"
#include "xml/xml.h"

#include "scip/scip_cons.h"
#include "scip/scip_copy.h"
#include "scip/scip_general.h"
#include "scip/scip_mem.h"
#include "scip/scip_message.h"
#include "scip/scip_nlp.h"
#include "scip/scip_numerics.h"
#include "scip/scip_param.h"
#include "scip/scip_prob.h"
#include "scip/scip_sol.h"
#include "scip/scip_solex.h"
#include "scip/scip_solve.h"
#include "scip/scip_solvingstats.h"
#include "scip/scip_var.h"

#include "scip/pub_cons.h"
#include "scip/pub_fileio.h"
#include "scip/pub_message.h"
#include "scip/pub_misc.h"
#include "scip/pub_sol.h"
#include "scip/pub_var.h"

#include "scip/struct_sol.h"


/* In debug mode, we include the SCIP's structure in scip.c, such that no one can access
 * this structure except the interface methods in scip.c.
 * In optimized mode, the structure is included in scip.h, because some of the methods
 * are implemented as defines for performance reasons (e.g. the numerical comparisons)
 */
#ifndef NDEBUG
#include "scip/struct_scip.h"
#endif

/** returns value of variable in primal CIP solution, or in current LP/pseudo solution
 *
 *  @return value of variable in primal CIP solution, or in current LP/pseudo solution
 *
 *  @pre In case the solution pointer @p sol is @b NULL, that means it is asked for the LP or pseudo solution, this method
 *       can only be called if @p scip is in the solving stage \ref SCIP_STAGE_SOLVING. In any other case, this method
 *       can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
void SCIPgetSolExVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOLEX*           sol,                /**< primal solution, or NULL for current LP/pseudo solution */
   SCIP_VAR*             var,                /**< variable to get value for */
   SCIP_Rational*        res                 /**< resulting rational */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolExVal", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   assert( var->scip == scip );

   if( sol != NULL )
      SCIPsolexGetVal(res, sol, scip->set, scip->stat, var);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolExVal(sol==NULL)", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   Rset(res, SCIPvarGetSolEx(var, SCIPtreeHasCurrentNodeLP(scip->tree)));
}

/** returns transformed objective value of primal CIP solution, or transformed current LP/pseudo objective value
 *
 *  @return transformed objective value of primal CIP solution, or transformed current LP/pseudo objective value
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
void SCIPgetSolExTransObj(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOLEX*           sol,                /**< primal solution, or NULL for current LP/pseudo objective value */
   SCIP_Rational*        res                 /**< result pointer to store rational */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolExTransObj", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   if( sol != NULL )
      Rset(res, SCIPsolexGetObj(sol, scip->set, scip->transprob, scip->origprob));
   else
   {
      SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolExTransObj(sol==NULL)", \
            FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );
      if( SCIPtreeHasCurrentNodeLP(scip->tree) )
         SCIPlpexGetObjval(scip->lpex, scip->set, scip->transprob, res);
      else
         SCIPlpexGetPseudoObjval(scip->lpex, scip->set, scip->transprob, res);
   }
}

SCIP_RETCODE SCIPoverwriteFPsol(
   SCIP*                 scip,
   SCIP_SOL*             sol,
   SCIP_SOLEX*           solex
   )
{
   int v;
   int nvars;
   SCIP_Rational* res;
   SCIP_VAR** vars;

   assert(scip != NULL);
   assert(sol != NULL);
   assert(solex != NULL);

   nvars = SCIPgetNVars(scip);
   vars = SCIPgetVars(scip);

   SCIP_CALL( RcreateTemp(SCIPbuffer(scip), &res) );

   for( v = 0; v < nvars; ++v )
   {
      SCIPgetSolExVal(scip, solex, vars[v], res);

      SCIP_CALL( SCIPsetSolVal(scip, sol, vars[v], RgetRealApprox(res)) );
   }

   SCIPgetSolExTransObj(scip, solex, res);
   SCIPsolSetObjVal(sol, RgetRealRelax(res, SCIP_ROUND_UPWARDS));

   RdeleteTemp(SCIPbuffer(scip), &res);

   return SCIP_OKAY;
}