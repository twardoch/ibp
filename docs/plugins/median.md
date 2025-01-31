# median

Smooths the image using a median filter

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_median](../assets/img_out/dog_median.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_median](../assets/img_out/female_median.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_median](../assets/img_out/male_median.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_median](../assets/img_out/tree_median.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.median
bypass=false
radius=75

[info]
description=Smooths the image using a median filter
fileType=ibp.imagefilterlist
nFilters=1
name=Median


```
