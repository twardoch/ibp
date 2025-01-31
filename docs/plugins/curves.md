# curves

Adjust the tonal curves of the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_curves](../assets/img_out/dog_curves.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_curves](../assets/img_out/female_curves.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_curves](../assets/img_out/male_curves.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_curves](../assets/img_out/tree_curves.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.curves
bypass=false
interpolationmode=normal
knots=3
workingchannel=1

[info]
description=Adjust the tonal curves of the image
fileType=ibp.imagefilterlist
nFilters=1
name=Curves


```
