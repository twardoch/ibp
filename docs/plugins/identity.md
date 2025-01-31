# identity

Just doesn't make any modifications to the image

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_identity](../assets/img_out/dog_identity.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_identity](../assets/img_out/female_identity.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_identity](../assets/img_out/male_identity.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_identity](../assets/img_out/tree_identity.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.identity
bypass=false

[info]
description=Just doesn't make any modifications to the image
fileType=ibp.imagefilterlist
nFilters=1
name=Identity


```
