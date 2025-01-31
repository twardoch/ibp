# bilateralfilter

Image filter plugin for bilateralfilter

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_bilateralfilter](../assets/img_out/dog_bilateralfilter.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_bilateralfilter](../assets/img_out/female_bilateralfilter.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_bilateralfilter](../assets/img_out/male_bilateralfilter.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_bilateralfilter](../assets/img_out/tree_bilateralfilter.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.bilateralfilter
bypass=false
edgepreservation=75
radius=7500.0

[info]
description=Image filter plugin for bilateralfilter
fileType=ibp.imagefilterlist
nFilters=1
name=Bilateral Filter


```
