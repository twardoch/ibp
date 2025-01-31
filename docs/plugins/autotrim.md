# autotrim

Trims the image based on some luma or alpha threshold value

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_autotrim](../assets/img_out/dog_autotrim.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_autotrim](../assets/img_out/female_autotrim.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_autotrim](../assets/img_out/male_autotrim.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_autotrim](../assets/img_out/tree_autotrim.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.autotrim
bypass=false
margins=75
reference=75
threshold=190

[info]
description=Trims the image based on some luma or alpha threshold value
fileType=ibp.imagefilterlist
nFilters=1
name=Auto Trim


```
