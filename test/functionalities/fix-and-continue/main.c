//===-- main.c --------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <stdio.h>

void f1(int);

int main (int argc, char const *argv[])
{
  f1(3); // Break here
  f1(3); // Step to here
  return 0;
}
