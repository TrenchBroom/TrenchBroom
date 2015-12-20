#!/bin/bash
/bin/cp -av /script/* .
make deb
make rpm
make tar
make movepkg
