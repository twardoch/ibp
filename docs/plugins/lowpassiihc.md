# lowpassiihc

Removes the artifacts due to a bad illumination using a low pass filtering approach

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_lowpassiihc](../assets/img_out/dog_lowpassiihc.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_lowpassiihc](../assets/img_out/female_lowpassiihc.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_lowpassiihc](../assets/img_out/male_lowpassiihc.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_lowpassiihc](../assets/img_out/tree_lowpassiihc.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.lowpassiihc
bypass=false
featuresize=150
outputmode=75

[info]
description=Removes the artifacts due to a bad illumination using a low pass filtering approach
fileType=ibp.imagefilterlist
nFilters=1
name=Low Pass IIH Correction


```
