# morphologicaliihc

Removes the artifacts due to a bad illumination using a morphological approach

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_morphologicaliihc](../assets/img_out/dog_morphologicaliihc.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_morphologicaliihc](../assets/img_out/female_morphologicaliihc.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_morphologicaliihc](../assets/img_out/male_morphologicaliihc.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_morphologicaliihc](../assets/img_out/tree_morphologicaliihc.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.morphologicaliihc
bypass=false
featuresize=150
outputmode=1

[info]
description=Removes the artifacts due to a bad illumination using a morphological approach
fileType=ibp.imagefilterlist
nFilters=1
name=Morphological IIH Correction


```
