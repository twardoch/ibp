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
highlightsblue=50
highlightsgreen=50
highlightsred=50
midtonesblue=50
midtonesgreen=50
midtonesred=50
preserveluminosity=1
shadowsblue=50
shadowsgreen=50
shadowsred=50

[info]
description=Image filter plugin for colorbalance
fileType=ibp.imagefilterlist
nFilters=1
name=Color Balance


```
