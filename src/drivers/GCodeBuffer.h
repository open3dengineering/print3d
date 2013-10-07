#ifndef GCODE_BUFFER_H_SEEN
#define GCODE_BUFFER_H_SEEN

#include <string>

class GCodeBuffer {
public:
	GCodeBuffer();

  void set(const std::string &gcode);
  void append(const std::string &gcode);
  void clear();

	int32_t getCurrentLine() const;
	int32_t getBufferedLines() const;
	int32_t getTotalLines() const;
	const std::string &getBuffer() const;
	int32_t getBufferSize() const;

	void setCurrentLine(int32_t line);

	bool getNextLine(std::string &line) const;
	bool eraseLine();

private:
	std::string buffer_;
	int32_t currentLine_;
	int32_t bufferedLines_;
	int32_t totalLines_;

	void updateStats(size_t pos);
	void cleanupGCode(size_t pos);
};

#endif /* ! GCODE_BUFFER_H_SEEN */
