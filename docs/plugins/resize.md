# resize

Resize the image without resampling its data

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_resize](../assets/img_out/dog_resize.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_resize](../assets/img_out/female_resize.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_resize](../assets/img_out/male_resize.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_resize](../assets/img_out/tree_resize.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.resize
bypass=false
anchorposition=2
backgroundcolor=0
height=90
heightmode=1
resizemode=normal
width=80
widthmode=1

[info]
description=Resize the image without resampling its data
fileType=ibp.imagefilterlist
nFilters=1
name=Resize


```
