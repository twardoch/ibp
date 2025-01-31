# threshold

Transforms the channels of the image into binary using a threshold value

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_threshold](../assets/img_out/dog_threshold.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_threshold](../assets/img_out/female_threshold.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_threshold](../assets/img_out/male_threshold.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_threshold](../assets/img_out/tree_threshold.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.threshold
bypass=false
affectedchannel=1
colormode=2
threshold=75

[info]
description=Transforms the channels of the image into binary using a threshold value
fileType=ibp.imagefilterlist
nFilters=1
name=Threshold


```
