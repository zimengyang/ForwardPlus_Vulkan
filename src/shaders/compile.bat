@echo off
glslangvalidator -V triangle.vert -o triangle.vert.spv
glslangvalidator -V triangle.frag -o triangle.frag.spv
glslangvalidator -V axis.vert -o axis.vert.spv
glslangvalidator -V axis.frag -o axis.frag.spv
glslangvalidator -V quad.frag -o quad.frag.spv
glslangvalidator -V computeLightList.comp -o computeLightList.comp.spv
glslangvalidator -V computeFrustumGrid.comp -o computeFrustumGrid.comp.spv
pause
