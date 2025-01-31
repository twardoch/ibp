# autothreshold

Transforms the channels of the image into binary using an automatic threshold value

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_autothreshold](../assets/img_out/dog_autothreshold.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_autothreshold](../assets/img_out/female_autothreshold.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_autothreshold](../assets/img_out/male_autothreshold.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_autothreshold](../assets/img_out/tree_autothreshold.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.autothreshold
bypass=false
affectedchannel=1
colormode=75
thresholdmode=75

[info]
description=Transforms the channels of the image into binary using an automatic threshold value
fileType=ibp.imagefilterlist
nFilters=1
name=Auto Threshold


```
