# basicrotation

Rotate the image by 90° increments

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_basicrotation](../assets/img_out/dog_basicrotation.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_basicrotation](../assets/img_out/female_basicrotation.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_basicrotation](../assets/img_out/male_basicrotation.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_basicrotation](../assets/img_out/tree_basicrotation.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.basicrotation
bypass=false
angle=75

[info]
description=Rotate the image by 90° increments
fileType=ibp.imagefilterlist
nFilters=1
name=Basic Rotation


```
