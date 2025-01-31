# boxblur

Smooths the image using a normalized box filter

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_boxblur](../assets/img_out/dog_boxblur.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_boxblur](../assets/img_out/female_boxblur.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_boxblur](../assets/img_out/male_boxblur.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_boxblur](../assets/img_out/tree_boxblur.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.boxblur
bypass=false
radius=50

[info]
description=Smooths the image using a normalized box filter
fileType=ibp.imagefilterlist
nFilters=1
name=Box Blur (Mean Filter)


```
