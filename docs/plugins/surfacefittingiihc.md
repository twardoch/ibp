# surfacefittingiihc

Removes the artifacts due to a bad illumination using a surface fitting approach

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_surfacefittingiihc](../assets/img_out/dog_surfacefittingiihc.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_surfacefittingiihc](../assets/img_out/female_surfacefittingiihc.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_surfacefittingiihc](../assets/img_out/male_surfacefittingiihc.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_surfacefittingiihc](../assets/img_out/tree_surfacefittingiihc.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.surfacefittingiihc
bypass=false
outputmode=25

[info]
description=Removes the artifacts due to a bad illumination using a surface fitting approach
fileType=ibp.imagefilterlist
nFilters=1
name=Surface Fitting IIH Correction


```
