@echo off
glslangvalidator -V final_shading.vert -o final_shading.vert.spv
glslangvalidator -V final_shading.frag -o final_shading.frag.spv
glslangvalidator -V axis.vert -o axis.vert.spv
glslangvalidator -V axis.frag -o axis.frag.spv
glslangvalidator -V quad.frag -o quad.frag.spv
glslangvalidator -V computeLightList.comp -o computeLightList.comp.spv
glslangvalidator -V computeFrustumGrid.comp -o computeFrustumGrid.comp.spv
pause
