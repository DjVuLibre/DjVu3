#!/bin/sh

mkdirp()
{
  if [ -n "$*" -a "$*" != "." -a ! -d "$*" ]
  then
    mkdirp `dirname "$*"`
    mkdir "$*"
  fi
}

mkdirp "$*"

