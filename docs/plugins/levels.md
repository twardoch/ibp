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
inputblackpoint=15.0
inputgamma=25.0
inputwhitepoint=35.0
outputblackpoint=45.0
outputwhitepoint=55.0
workingchannel=1

[info]
description=Adjust the black, mid and white tones of the image
fileType=ibp.imagefilterlist
nFilters=1
name=Levels


```
