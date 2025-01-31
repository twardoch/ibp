# autolevels

Adjust the levels automatically

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_autolevels](../assets/img_out/dog_autolevels.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_autolevels](../assets/img_out/female_autolevels.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_autolevels](../assets/img_out/male_autolevels.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_autolevels](../assets/img_out/tree_autolevels.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.autolevels
bypass=false
adjustmidtones=1
clippinghighlights=50
clippingshadows=60
enhancechannelsseparately=1
targetcolorhighlights=20
targetcolormidtones=30
targetcolorshadows=40

[info]
description=Adjust the levels automatically
fileType=ibp.imagefilterlist
nFilters=1
name=Auto Levels


```
