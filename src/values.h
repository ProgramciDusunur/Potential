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


#endif //POTENTIAL_VALUES_H
