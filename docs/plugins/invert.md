# invert

Invert one or more channels of the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_invert](../assets/img_out/dog_invert.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_invert](../assets/img_out/female_invert.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_invert](../assets/img_out/male_invert.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_invert](../assets/img_out/tree_invert.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.invert
bypass=false
alphachannel=1
bluechannel=0
greenchannel=1
redchannel=0

[info]
description=Invert one or more channels of the image
fileType=ibp.imagefilterlist
nFilters=1
name=Invert


```
