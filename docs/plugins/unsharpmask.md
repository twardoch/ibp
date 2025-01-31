# unsharpmask

Enhances the image by contrasting fine details

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_unsharpmask](../assets/img_out/dog_unsharpmask.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_unsharpmask](../assets/img_out/female_unsharpmask.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_unsharpmask](../assets/img_out/male_unsharpmask.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_unsharpmask](../assets/img_out/tree_unsharpmask.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.unsharpmask
bypass=false
amount=375
radius=7500.0
threshold=191

[info]
description=Enhances the image by contrasting fine details
fileType=ibp.imagefilterlist
nFilters=1
name=Unsharp Mask


```
