#include <vector>
#include <stdexcept>
#include <numeric>
#include <random>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
//#include <boost/asio.hpp>

#include "utils.hpp"

namespace rota
{
    int weightedChoice(std::vector<float> *weights){
        float weightSum = std::accumulate(weights->begin(), weights->end(), 0);
        weightSum = roundf(weightSum * 100) / 100; // round to 2 decimal places to account floating point error
        if (weightSum != 1){
            throw std::invalid_argument("Weights do not sum to 1");
        }
        float randomValue = (float)rand()/RAND_MAX;
        float currentValue = 0;
        for(int i=0; i<weights->size(); i++){
            currentValue += weights->at(i);
            if(randomValue <= currentValue){
                return i;
            }
        }
        return -1;
    }

    RotaLayer *getLayers(std::string url, std::string req){

        /*
        using boostTcp = boost::asio::ip::tcp;

        try
        {
            boost::asio::io_service io_service;

            // Get a list of endpoints corresponding to the server name.
            boostTcp::tcp::resolver resolver(io_service);
            boostTcp::tcp::resolver::query query(url, "http");
            boostTcp::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

            // Try each endpoint until we successfully establish a connection.
            boostTcp::tcp::socket socket(io_service);
            boost::asio::connect(socket, endpoint_iterator);

            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "GET " << req << " HTTP/1.0\r\n";
            request_stream << "Host: " << url << "\r\n";
            request_stream << "Accept: *\r\n";
            request_stream << "Connection: close\r\n\r\n";

            // Send the request.
            boost::asio::write(socket, request);

            // Read the response status line. The response streambuf will automatically
            // grow to accommodate the entire line. The growth may be limited by passing
            // a maximum size to the streambuf constructor.
            boost::asio::streambuf response;
            boost::asio::read_until(socket, response, "\r\n");

            // Check that response is OK.
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
            std::cout << "Invalid response\n";
            return nullptr;
            }
            if (status_code != 200)
            {
            std::cout << "Response returned with status code " << status_code << "\n";
            return nullptr;
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::read_until(socket, response, "\r\n\r\n");

            // Process the response headers.
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
            std::cout << header << "\n";
            std::cout << "\n";

            // Write whatever content we already have to output.
            if (response.size() > 0)
            std::cout << &response;

            // Read until EOF, writing data to output as we go.
            boost::system::error_code error;
            while (boost::asio::read(socket, response,
                boost::asio::transfer_at_least(1), error))
            std::cout << &response;
            if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << "\n";
        }

        return 0;
        */

       return nullptr;
    }


} // namespace rota
