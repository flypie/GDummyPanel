/*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once


class Button
{
	friend class ComplexWindow;

public:
	Button();
	Button(int inx, int  iny, int  inw, int  inh, int  ini);


	void draw();
	void SetSelected(bool In);

	void SetOutput(bool In);
	void SetInput(bool In);
	void SetEnabled(bool In);

	bool GetSelected();
	bool	GetOut();
	int		GetiData();

private:

	int		x, y, w, h;
	char	*Text;
	char	*BG;
	bool	Selected;
	Button *Next;
	int		iData;
	WINDOW	*Win;

	bool	Out;
	bool	In;
	bool	Enabled;
};



