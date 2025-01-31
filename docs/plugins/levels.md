# levels

Adjust the black, mid and white tones of the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_levels](../assets/img_out/dog_levels.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_levels](../assets/img_out/female_levels.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_levels](../assets/img_out/male_levels.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_levels](../assets/img_out/tree_levels.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.levels
bypass=false
inputblackpoint=75.0
inputgamma=75.0
inputwhitepoint=75.0
outputblackpoint=75.0
outputwhitepoint=75.0
workingchannel=75

[info]
description=Adjust the black, mid and white tones of the image
fileType=ibp.imagefilterlist
nFilters=1
name=Levels


```
