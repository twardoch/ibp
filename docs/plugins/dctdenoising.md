# dctdenoising

Image filter plugin for dctdenoising

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_dctdenoising](../assets/img_out/dog_dctdenoising.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_dctdenoising](../assets/img_out/female_dctdenoising.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_dctdenoising](../assets/img_out/male_dctdenoising.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_dctdenoising](../assets/img_out/tree_dctdenoising.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.dctdenoising
bypass=false
strength=75

[info]
description=Image filter plugin for dctdenoising
fileType=ibp.imagefilterlist
nFilters=1
name=DCT Denoising


```
