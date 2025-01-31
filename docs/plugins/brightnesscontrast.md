# brightnesscontrast

Adjust levels of the image in an easy way

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_brightnesscontrast](../assets/img_out/dog_brightnesscontrast.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_brightnesscontrast](../assets/img_out/female_brightnesscontrast.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_brightnesscontrast](../assets/img_out/male_brightnesscontrast.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_brightnesscontrast](../assets/img_out/tree_brightnesscontrast.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.brightnesscontrast
bypass=false
brightness=30
contrast=20
usesoftmode=1
workingchannel=2

[info]
description=Adjust levels of the image in an easy way
fileType=ibp.imagefilterlist
nFilters=1
name=Brightness and Contrast


```
