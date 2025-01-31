# colorbalance

Image filter plugin for colorbalance

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_colorbalance](../assets/img_out/dog_colorbalance.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_colorbalance](../assets/img_out/female_colorbalance.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_colorbalance](../assets/img_out/male_colorbalance.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_colorbalance](../assets/img_out/tree_colorbalance.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.colorbalance
bypass=false
highlightsblue=20
highlightsgreen=30
highlightsred=40
midtonesblue=50
midtonesgreen=60
midtonesred=70
preserveluminosity=1
shadowsblue=80
shadowsgreen=10
shadowsred=50

[info]
description=Image filter plugin for colorbalance
fileType=ibp.imagefilterlist
nFilters=1
name=Color Balance


```
