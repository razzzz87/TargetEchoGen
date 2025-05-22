#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/init.h>

// Dummy I2C device address
#define DUMMY_I2C_ADDR 0x50

static int dummy_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    pr_info("Dummy I2C driver probed\n");
    return 0;
}

static int dummy_remove(struct i2c_client *client)
{
    pr_info("Dummy I2C driver removed\n");
    return 0;
}

static const struct i2c_device_id dummy_id[] = {
    { "dummy_i2c", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, dummy_id);

static struct i2c_driver dummy_driver = {
    .driver = {
        .name = "dummy_i2c",
    },
    .probe = dummy_probe,
    .remove = dummy_remove,
    .id_table = dummy_id,
};

module_i2c_driver(dummy_driver);

MODULE_AUTHOR("Raj Kishor");
MODULE_DESCRIPTION("I2C Driver");
MODULE_LICENSE("GPL");

