# huesaturation

Change the color information of the image in the HSL space

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_huesaturation](../assets/img_out/dog_huesaturation.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_huesaturation](../assets/img_out/female_huesaturation.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_huesaturation](../assets/img_out/male_huesaturation.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_huesaturation](../assets/img_out/tree_huesaturation.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.huesaturation
bypass=false
abshue=270
abssaturation=75
colorize=1
relhue=90
rellightness=50
relsaturation=50

[info]
description=Change the color information of the image in the HSL space
fileType=ibp.imagefilterlist
nFilters=1
name=Hue/Saturation/Lightness


```
