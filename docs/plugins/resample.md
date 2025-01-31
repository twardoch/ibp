# resample

Resample the image to a new size using the selected resampling interpolation mode

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_resample](../assets/img_out/dog_resample.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_resample](../assets/img_out/female_resample.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_resample](../assets/img_out/male_resample.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_resample](../assets/img_out/tree_resample.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.resample
bypass=false
height=200
heightmode=1
resamplingmode=normal
width=200
widthmode=1

[info]
description=Resample the image to a new size using the selected resampling interpolation mode
fileType=ibp.imagefilterlist
nFilters=1
name=Resample


```
