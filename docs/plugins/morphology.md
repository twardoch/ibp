# morphology

Apply a basic mathematical morphology operation to the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_morphology](../assets/img_out/dog_morphology.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_morphology](../assets/img_out/female_morphology.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_morphology](../assets/img_out/male_morphology.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_morphology](../assets/img_out/tree_morphology.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.morphology
bypass=false
hradius=30
kernelshape=normal
lockradius=1
modifyalpha=0
modifyrgb=1
morphologyop=20
vradius=40

[info]
description=Apply a basic mathematical morphology operation to the image
fileType=ibp.imagefilterlist
nFilters=1
name=Basic Morphology


```
