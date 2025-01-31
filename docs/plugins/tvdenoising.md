# tvdenoising

Removes the noise from the image using a variational method

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_tvdenoising](../assets/img_out/dog_tvdenoising.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_tvdenoising](../assets/img_out/female_tvdenoising.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_tvdenoising](../assets/img_out/male_tvdenoising.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_tvdenoising](../assets/img_out/tree_tvdenoising.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.tvdenoising
bypass=false
iterations=75
strength=7500.0

[info]
description=Removes the noise from the image using a variational method
fileType=ibp.imagefilterlist
nFilters=1
name=Total Variation Denoising


```
