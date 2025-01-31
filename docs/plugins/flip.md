# flip

Flip the image around the vertical and/or horizontal axis

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_flip](../assets/img_out/dog_flip.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_flip](../assets/img_out/female_flip.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_flip](../assets/img_out/male_flip.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_flip](../assets/img_out/tree_flip.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.flip
bypass=false
direction=1

[info]
description=Flip the image around the vertical and/or horizontal axis
fileType=ibp.imagefilterlist
nFilters=1
name=Flip


```
