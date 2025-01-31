# colorlayer

Adds a color layer to the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_colorlayer](../assets/img_out/dog_colorlayer.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_colorlayer](../assets/img_out/female_colorlayer.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_colorlayer](../assets/img_out/male_colorlayer.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_colorlayer](../assets/img_out/tree_colorlayer.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.colorlayer
bypass=false
color=20
colorcompositionmode=1
opacity=2
position=33

[info]
description=Adds a color layer to the image
fileType=ibp.imagefilterlist
nFilters=1
name=Color Layer


```
