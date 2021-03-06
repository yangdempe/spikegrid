// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// A sample program demonstrating using Google C++ testing framework.
//
// Author: wan@google.com (Zhanyong Wan)


// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.
//
// Writing a unit test using Google C++ testing framework is easy as 1-2-3:


// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <limits.h>
#include "gtest/gtest.h"

#define TESTING
extern "C"
{
#include "../sizes.h"
#include "../STDP.h"
#include "../evolve.h"
#include "../coupling.h"
#include "consts.h"
}

//first param is group - second is test name
TEST(STDP,wrap)
{
    EXPECT_EQ(0,wrap(0));
    EXPECT_EQ(0,wrap(grid_size));
    EXPECT_EQ(grid_size-1,wrap(-1));
    EXPECT_EQ(1,wrap(grid_size+1));
}
TEST(STDP,clamp)
{
    EXPECT_DOUBLE_EQ(0,clamp(0,0,0));
    EXPECT_DOUBLE_EQ(0,clamp(9,0,0));
    EXPECT_DOUBLE_EQ(0,clamp(-9,0,0));
    EXPECT_DOUBLE_EQ(1,clamp(1,1,1));
    EXPECT_DOUBLE_EQ(1,clamp(1.1,1,1));
    EXPECT_DOUBLE_EQ(0.05,clamp(1.1,1,0.05));
}
TEST(evolve,ActiveNeuron)
{
    //with skip of 1, all neurons are active
    EXPECT_TRUE(IsActiveNeuron(0,0,1));
    EXPECT_TRUE(IsActiveNeuron(50,12,1));
    EXPECT_TRUE(IsActiveNeuron(50,10,1));
    //with +ve skip, some things are skipped
    EXPECT_TRUE(IsActiveNeuron(10,10,2));
    EXPECT_FALSE(IsActiveNeuron(10,9,2));
    //and with -ve skip we get the inverse of +ve skip
    EXPECT_FALSE(IsActiveNeuron(10,10,-2));
    EXPECT_TRUE(IsActiveNeuron(10,9,-2));
}
TEST(coupling,NonzeroCouplings)
{
    double* d;
    int c;
    Non_zerocouplings(couple_example,&d,&c);
    //the assertion here checks the count of non zero couplings.
    //we use some bindings on PI to account for grid-induced rounding
    EXPECT_LT(3*couplerange*couplerange,c);
    EXPECT_GT(4*couplerange*couplerange,c);

}
