//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_VALUES_H
#define POTENTIAL_VALUES_H

#pragma once

/*  these are the score bounds for the range of the mating scores
                                        Score layot
    [-infinity, mateValue ... mateScore ... score ... mateScore ... mateValue, infinity]
 */
#define infinity  32000
#define mateValue 31000
#define mateScore 30000
#define noEval 200000
#define noScore -infinity


#endif //POTENTIAL_VALUES_H
