# equalize

Automatically equalize the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_equalize](../assets/img_out/dog_equalize.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_equalize](../assets/img_out/female_equalize.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_equalize](../assets/img_out/male_equalize.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_equalize](../assets/img_out/tree_equalize.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.equalize
bypass=false

[info]
description=Automatically equalize the image
fileType=ibp.imagefilterlist
nFilters=1
name=Equalize


```
