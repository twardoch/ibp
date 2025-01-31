# nlmdenoising

Removes the noise from the image using semi-local information

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_nlmdenoising](../assets/img_out/dog_nlmdenoising.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_nlmdenoising](../assets/img_out/female_nlmdenoising.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_nlmdenoising](../assets/img_out/male_nlmdenoising.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_nlmdenoising](../assets/img_out/tree_nlmdenoising.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.nlmdenoising
bypass=false
strength=75

[info]
description=Removes the noise from the image using semi-local information
fileType=ibp.imagefilterlist
nFilters=1
name=Non-Local Means Denoising


```
