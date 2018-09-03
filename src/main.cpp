#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <CL/cl.h>
#include <libclew/ocl_init.h>

class OCL_Exception : public std::runtime_error
{
public:
  OCL_Exception(const std::string message, cl_int errcode) : 
    std::runtime_error(message), errcode(errcode) {}

  cl_int getErrorCode() const { return errcode; }

private:
  cl_int errcode;
};

template <typename T>
std::string to_string(T value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

void reportError(cl_int err, const std::string &filename, int line)
{
    if (CL_SUCCESS == err)
        return;

    std::string message = "OpenCL error code " + to_string(err) + " encountered at " + filename + ":" + to_string(line);
    throw OCL_Exception(message, err);
}

#define OCL_SAFE_CALL(expr) reportError(expr, __FILE__, __LINE__)

// For future reference:
// https://www.khronos.org/registry/OpenCL/sdk/1.2/docs/man/xhtml/

int main()
{
    if (!ocl_init())
        throw std::runtime_error("Can't init OpenCL driver!");

    cl_uint platformsCount = 0;
    OCL_SAFE_CALL(clGetPlatformIDs(0, nullptr, &platformsCount));
    std::cout << "Number of OpenCL platforms: " << platformsCount << std::endl;

    std::vector<cl_platform_id> platforms(platformsCount);
    OCL_SAFE_CALL(clGetPlatformIDs(platformsCount, platforms.data(), nullptr));

    // TODO 1.1
    // Попробуйте вместо CL_PLATFORM_NAME передать какое-нибудь случайное число - например 239
    // Т.к. это некорректный идентификатор параметра платформы - то метод вернет код ошибки
    // Макрос OCL_SAFE_CALL заметит это, и кинет ошибку с кодом
    // Откройте таблицу с кодами ошибок:
    // libs/clew/CL/cl.h:103
    // P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже с номером строки: cl.h:103) -> Enter
    // Найдите там нужный код ошибки и ее название
    // Затем откройте документацию по clGetPlatformInfo и в секции Errors найдите ошибку, с которой столкнулись
    // в документации подробно объясняется, какой ситуации соответствует данная ошибка, и это позволит проверив код понять чем же вызвана данная ошибка (не корректным аргументом param_name)
    // Обратите внимание что в этом же libs/clew/CL/cl.h файле указаны всевоможные defines такие как CL_DEVICE_TYPE_GPU и т.п.
    try
    {
      std::cout << "Invoking clGetPlatformInfo with incorrect parameter...\n";
      constexpr cl_uint INCORRECT_PARAM_NAME = 239;
      std::size_t paramValueSize;
      OCL_SAFE_CALL(clGetPlatformInfo(platforms[0], INCORRECT_PARAM_NAME, 0, nullptr, &paramValueSize));
    }
    catch (const OCL_Exception& e)
    {
      std::cout << "OCL exception caught, errorcode: " << e.getErrorCode() << "\n\n";
    }

    for (auto platformIndex = 0u; platformIndex < platformsCount; ++platformIndex) {
        std::cout << "Platform #" << (platformIndex + 1) << "/" << platformsCount << std::endl;
        cl_platform_id platform = platforms[platformIndex];

        size_t platformNameSize = 0;
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, nullptr, &platformNameSize));

        // TODO 1.2
        // Аналогично тому как был запрошен список идентификаторов всех платформ - так и с названием платформы, теперь, когда известна длина названия - его можно запросить:
        std::vector<unsigned char> platformName(platformNameSize, 0);
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_NAME, platformName.size(), platformName.data(), nullptr));
        std::cout << "    Platform name: " << platformName.data() << std::endl;

        // TODO 1.3
        // Запросите и напечатайте так же в консоль вендора данной платформы
        size_t platformVendorSize;
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0, nullptr, &platformVendorSize));
        std::vector<char> platformVendor(platformVendorSize, 0);
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, platformVendor.size(), platformVendor.data(), nullptr));
        std::cout << "    Platform vendor: " << platformVendor.data() << std::endl;

        // TODO 2.1
        // Запросите число доступных устройств данной платформы (аналогично тому как это было сделано для запроса числа доступных платформ - см. секцию "OpenCL Runtime" -> "Query Devices")
        cl_uint devicesCount;
        OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &devicesCount));
        std::cout << "    Devices available: " << devicesCount << std::endl;

        std::vector<cl_device_id> devices(devicesCount);
        OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, devices.size(), devices.data(), nullptr));

        for (auto deviceIndex = 0u; deviceIndex < devicesCount; ++deviceIndex) {
            // TODO 2.2
            // Запросите и напечатайте в консоль:
            // - Название устройства
            // - Тип устройства (видеокарта/процессор/что-то странное)
            // - Размер памяти устройства в мегабайтах
            // - Еще пару или более свойств устройства, которые вам покажутся наиболее интересными
            cl_device_id device = devices[deviceIndex];

            std::size_t deviceNameSize;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &deviceNameSize));
            std::vector<char> deviceName(deviceNameSize, 0);
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_NAME, deviceNameSize, deviceName.data(), nullptr));

            cl_device_type deviceType;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, nullptr));

            cl_ulong deviceGlobalMemorySize;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &deviceGlobalMemorySize, nullptr));

            cl_ulong deviceLocalMemorySize;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &deviceLocalMemorySize, nullptr));

            cl_bool isAvailable;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_AVAILABLE, sizeof(cl_bool), &isAvailable, nullptr));

            auto mapDeviceTypeToStr = [](cl_device_type type) {
                if (type & CL_DEVICE_TYPE_CPU)
                    return "CPU";
                else if (type & CL_DEVICE_TYPE_GPU)
                    return "GPU";
                else
                    return "Unknown";
            };

            std::cout
                << "    Device #"   << deviceIndex + 1 << "/#" << devices.size() << '\n'
                << "        Name: " << deviceName.data() << '\n'
                << "        Type: " << mapDeviceTypeToStr(deviceType) << '\n'
                << "        Global memory size: " << (deviceGlobalMemorySize / (1 << 20)) << " MB\n"
                << "        Local memory size: "  << (deviceLocalMemorySize / (1 << 10)) << " KB\n"
                << "        Available: " << (isAvailable ? "Yes" : "No") << '\n';
        }
    }

    return 0;
}