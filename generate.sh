#!/bin/bash

SpecCompiler=$HOME/spec-checker-compiler

ClassPath=$SpecCompiler/classes

Class=edu/uci/eecs/codeGenerator/CodeGenerator 

java -cp $ClassPath $Class $1
