//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_VALUES_H
#define POTENTIAL_VALUES_H

#pragma once

/*  these are the score bounds for the range of the mating scores
                                        Score layot
    [-infinity, mateValue ... score ... mateValue, infinity]
 */

enum {
   mateValue = 31000,
   infinity = 32000,
   mateFound = mateValue - maxPly,
   noEval = 200000
};

enum {
   STACK_OFFSET = 10,
   STACK_SAFETY_MARGIN = 20
};

enum {
   STAGE_TT,
   STAGE_GEN_NOISY,
   STAGE_GOOD_NOISY,
   STAGE_GEN_QUIET,
   STAGE_QUIET,
   STAGE_BAD_NOISY,
   CURRENT_STAGE
};

enum {
   STAGE_QS_GEN_NOISY,
   STAGE_QS_NOISY
};

enum {
   STAGE_PROBCUT_TT,
   STAGE_PROBCUT_GEN_NOISY,
   STAGE_PROBCUT_NOISY
};


#endif //POTENTIAL_VALUES_H
