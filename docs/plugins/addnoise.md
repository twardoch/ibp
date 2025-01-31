# addnoise

Add uniform or gaussian distributed noise to the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_addnoise](../assets/img_out/dog_addnoise.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_addnoise](../assets/img_out/female_addnoise.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_addnoise](../assets/img_out/male_addnoise.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_addnoise](../assets/img_out/tree_addnoise.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.addnoise
bypass=false
amount=30000.0
colormode=#FF0000
distribution=75
seed=75

[info]
description=Add uniform or gaussian distributed noise to the image
fileType=ibp.imagefilterlist
nFilters=1
name=Add Noise


```
