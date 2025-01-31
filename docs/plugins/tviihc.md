# tviihc

Image filter plugin for tviihc

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_tviihc](../assets/img_out/dog_tviihc.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_tviihc](../assets/img_out/female_tviihc.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_tviihc](../assets/img_out/male_tviihc.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_tviihc](../assets/img_out/tree_tviihc.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.tviihc
bypass=false
outputmode=75
refinement=75
smoothness=75

[info]
description=Image filter plugin for tviihc
fileType=ibp.imagefilterlist
nFilters=1
name=Tina Vision IIH Correction


```
