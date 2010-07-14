# -*- coding: utf-8 -*-
# $Id$

import sys
import nuto
import os
import math

#if set to true, the result will be generated (for later use in the test routine)
#otherwise, the current result will be compared to the stored result
createResult = False

#show the results on the screen
printResult = False

#system name and processor
system = sys.argv[1]+sys.argv[2]

#path in the original source directory and current filename at the and
pathToResultFiles = os.path.join(sys.argv[3],"results",system,os.path.basename(sys.argv[0]))

#remove the extension
fileExt = os.path.splitext(sys.argv[0])[1]
pathToResultFiles = pathToResultFiles.replace(fileExt,'')

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#% no error in file, modified, if error is detected              %
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
error = False

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#% start the real test file                                      %
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
structure=nuto.Structure(3)

Section1 = structure.SectionCreate("Truss")
structure.SectionSetArea(Section1, 3)
area = structure.SectionGetArea(Section1)
if(printResult):
    structure.SectionInfo(Section1, 0)
if(area != 3):
    print '[' + system,sys.argv[0] + '] : cross-section area is not correct.'
    error = True

Section2 = structure.SectionCreate("Plane_Strain")
structure.SectionSetThickness(Section2, 5)
thickness = structure.SectionGetThickness(Section2)
if(printResult):
    structure.SectionInfo(Section2, 0)
if(thickness != 5):
    print '[' + system,sys.argv[0] + '] : section thickness is not correct.'
    error = True

if (error):
    sys.exit(-1)
else:
    sys.exit(0)
