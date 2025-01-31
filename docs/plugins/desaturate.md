# desaturate

Removes the chroma information of the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_desaturate](../assets/img_out/dog_desaturate.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_desaturate](../assets/img_out/female_desaturate.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_desaturate](../assets/img_out/male_desaturate.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_desaturate](../assets/img_out/tree_desaturate.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.desaturate
bypass=false

[info]
description=Removes the chroma information of the image
fileType=ibp.imagefilterlist
nFilters=1
name=Desaturate


```
